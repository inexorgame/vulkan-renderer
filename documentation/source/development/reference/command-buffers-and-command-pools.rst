Management of command pools and command bufers
==============================================

Inexor engine's command pool and command buffer management code is based on the following resources:

* `Tips and Tricks: Vulkan Dos and Don'ts  <https://developer.nvidia.com/blog/vulkan-dos-donts/>`__
* `Writing an efficient Vulkan renderer <https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/>`__
* `Themaister's Granite renderer <https://github.com/Themaister/Granite>`__
* `Multi-Threading in Vulkan <https://community.arm.com/arm-community-blogs/b/graphics-gaming-and-vr-blog/posts/multi-threading-in-vulkan>`__

From that we concluded our solution should look like this:

* The device wrapper should be exclusively responsible for the management of command pools and command buffers
* The command pool and command buffer management system must be thread safe
* We should have only one command pool per thread per queue (as is recommended)
* Each command buffer should be allocated from the command pool which is associated to the current thread
* Command buffers must be reused instead of being allocated and destroyed every time
* We should abstract command buffers as much as possible
* The solution must support an arbitrary number of threads and an arbitrary number of command buffers per thread
* Command buffer recording should be done with the ``VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`` flag
* We should start with graphics command pools only and work on other pool types (transfer, compute..) later (GitHub issue https://github.com/inexorgame/vulkan-renderer/issues/486)

.. note::
    Do not forget that we need to specify on which queue the command pool will operate on. We are currently using the graphics queue for copy operations, but there are other queue types such as transfer queues or compute queues. We will need to create separate command pools on a per-thread basis once we want to use those queues.

Thread-local command pools
--------------------------

The command pools are managed exclusively by the device wrapper class. Inside of it, there is a std::vector of std::unique_ptr of the CommandPool wrapper class:

.. code-block:: cpp

    std::vector<std::unique_ptr<CommandPool>> m_cmd_pools;
    std::mutex m_mutex;

.. note::
    It must be a ``std::vector<std::unique_ptr<CommandPool>>`` and not a ``std::vector<CommandPool>`` because std::vector is allowed to reorder the memory and we want to store a pointer into the std::vector!

Since the command pools are owned by a thread-global device object, the vector must be protected by a mutex. But how do we set up a command pool on a per-thread basis and how do we get the command pool for the current thread? To do this, we have a non-owning ``thread_local`` pointer to the thread's command pool which points to an entry inside the device's command pool vector, and is initialized the first time the getter is called.

.. code-block:: cpp

    CommandPool &Device::thread_graphics_pool() {
        // Note that thread_graphics_pool is implicitely static!
        thread_local CommandPool *thread_graphics_pool = nullptr; // NOLINT
        if (thread_graphics_pool == nullptr) {
            auto cmd_pool = std::make_unique<CommandPool>(*this, "graphics pool");
            std::scoped_lock locker(m_mutex);
            thread_graphics_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
        }
        return *thread_graphics_pool;
    }

This get method is private. It is used only internally for the command buffer request system, as explained in the next section.

Command buffer request system
-----------------------------

Inexor engine uses a command buffer request system. If you need to record and submit a command buffer in any place in the engine code, you can call ``m_device.request_command_buffer()``. You should have a reference to the device wrapper ``m_device`` available in the part of the code you want to use the command buffer. The command buffers are managed by the command pool wrapper and can only be accessed through the the wrapper. However, the command pools themselves are managed by the device wrapper. This means command pools are never directly exposed in the rest of the engine code. You request a command buffer from the device wrapper, and the request will be redirected internally to the thread local command pool:

.. code-block:: cpp

    const CommandBuffer &Device::request_command_buffer(const std::string &name) {
        return thread_graphics_pool().request_command_buffer(name);
    }

The request method of the command pool wrapper tries to find a command buffer which is currently not used anywhere else. It does so by testing the state of the command buffer's fence. If no free command buffer is found, a new one is simply allocated. Note that this is thread local, so we need no synchronization here. Note that the command buffer request method resets the command buffer's fence:

.. code-block:: cpp

    const CommandBuffer &CommandPool::request_command_buffer(const std::string &name) {
       // Try to find a command buffer which is currently not used
       for (const auto &cmd_buf : m_cmd_bufs) {
           if (cmd_buf->fence_status() == VK_SUCCESS) {
                // Reset the command buffer's fence to make it usable again
                cmd_buf->reset_fence();
                m_device.set_debug_marker_name(*cmd_buf->ptr(), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, name);
                return *cmd_buf;
            }
       }
       // We need to create a new command buffer because no free one was found
       // Note that there is currently no method for shrinking m_cmd_bufs, but this should not be a problem
       m_cmd_bufs.emplace_back(std::make_unique<CommandBuffer>(m_device, m_cmd_pool, "command buffer"));
       return *m_cmd_bufs.back();
    }

After this, you can use it to record and submit your command buffer. You can also use the ``execute`` method, as explained in the next section.

Device wrapper's execute method
-------------------------------

To automate beginning and ending of command buffer recording and submission, we created the execute method of the device wrapper. This is quire helpful and it is recommended to use it instead of requesting command buffer handles manually. The execute method takes a lambda as argument and calls ``begin_command_buffer`` before executing it. After execution, it calls ``end_command_buffer`` and ``submit_and_wait``. For debugging purposes, it also assigns a debug name to the command buffer which executes your lambda:

.. code-block:: cpp

    void Device::execute(const std::string &name, const std::function<void(const CommandBuffer &cmd_buf)> &cmd_lambda) {
       // TODO: Support other queues (not just graphics)
       const auto &cmd_buf = thread_graphics_pool().request_command_buffer(name);
       cmd_buf.begin_command_buffer();
       // Execute the lambda
       cmd_lambda(cmd_buf);
       cmd_buf.end_command_buffer().submit_and_wait();
    }

.. note::
    Note that ``execute`` will wait for the command buffer submission and execution to complete using a fence, meaning it's a blocking operation. In case you don't want this, you should be experienced enough to use the ``request_command_buffer`` method manually and to do your synchronization yourself.

Here is an example for an image copy operation which uses the execute method:

.. code-block:: cpp

    m_device.execute(m_name, [&](const CommandBuffer &cmd_buf) {
       cmd_buf.change_image_layout(m_texture_image->get(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
              .copy_buffer_to_image(texture_data, static_cast<VkDeviceSize>(texture_size), m_texture_image->get(), copy_region, m_name)
              .change_image_layout(m_texture_image->get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

.. note::
    Inexor engine's command buffer request system does not differentiate between a normal command buffer and a command buffer which is used for single submission. In fact, all command buffers have the ``VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`` when calling ``begin_command_buffer``.
