#pragma once

// TODO: Forward declare.
#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/fence.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// TODO: Compute stages.
// TODO: Uniform buffers.

namespace inexor::vulkan_renderer {

class PhysicalResource;
class PhysicalStage;
class RenderGraph;

/// @brief Base class of all render graph objects (resources and stages).
/// @note This is just for internal use.
struct RenderGraphObject {
    RenderGraphObject() = default;
    RenderGraphObject(const RenderGraphObject &) = delete;
    RenderGraphObject(RenderGraphObject &&) = delete;
    virtual ~RenderGraphObject() = default;

    RenderGraphObject &operator=(const RenderGraphObject &) = delete;
    RenderGraphObject &operator=(RenderGraphObject &&) = delete;

    /// @brief Casts this object to type `T`
    /// @return The object as type `T` or `nullptr` if the cast failed
    template <typename T>
    [[nodiscard]] T *as();

    /// @copydoc as
    template <typename T>
    [[nodiscard]] const T *as() const;
};

/// @brief A single resource in the render graph.
/// @note May become multiple physical (vulkan) resources during render graph compilation.
class RenderResource : public RenderGraphObject {
    friend RenderGraph;

private:
    const std::string m_name;
    std::shared_ptr<PhysicalResource> m_physical;

protected:
    explicit RenderResource(std::string name) : m_name(std::move(name)) {}

public:
    RenderResource(const RenderResource &) = delete;
    RenderResource(RenderResource &&) = delete;
    ~RenderResource() override = default;

    RenderResource &operator=(const RenderResource &) = delete;
    RenderResource &operator=(RenderResource &&) = delete;
};

enum class BufferUsage {
    /// @brief Specifies that the buffer will be used to input index data.
    INDEX_BUFFER,

    /// @brief Specifies that the buffer will be used to input per vertex data to a vertex shader.
    VERTEX_BUFFER,
};

class BufferResource : public RenderResource {
    friend RenderGraph;

private:
    const BufferUsage m_usage;
    std::vector<VkVertexInputAttributeDescription> m_vertex_attributes;

    // Data to upload during render graph compilation.
    const void *m_data{nullptr};
    std::size_t m_data_size{0};
    bool m_data_upload_needed{false};
    std::size_t m_element_size{0};

public:
    BufferResource(std::string &&name, BufferUsage usage) : RenderResource(name), m_usage(usage) {}

    /// @brief Specifies that element `offset` of this vertex buffer is of format `format`.
    /// @note Calling this function is only valid on buffers of type BufferUsage::VERTEX_BUFFER.
    void add_vertex_attribute(VkFormat format, std::uint32_t offset);

    /// @brief Specifies the element size of the buffer upfront if data is not to be uploaded immediately.
    /// @param element_size The element size in bytes
    void set_element_size(std::size_t element_size) {
        m_element_size = element_size;
    }

    /// @brief Specifies the data that should be uploaded to this buffer at the start of the next frame.
    /// @param count The number of elements (not bytes) to upload
    /// @param data A pointer to a contiguous block of memory that is at least `count * sizeof(T)` bytes long
    // TODO: Use std::span when we switch to C++ 20.
    template <typename T>
    void upload_data(const T *data, std::size_t count);

    /// @brief @copybrief upload_data(const T *, std::size_t)
    /// @note This is equivalent to doing `upload_data(data.data(), data.size() * sizeof(T))`
    /// @see upload_data(const T *data, std::size_t count)
    template <typename T>
    void upload_data(const std::vector<T> &data);
};

enum class TextureUsage {
    /// @brief Specifies that this texture is the output of the render graph.
    // TODO: Refactor back buffer system more (remove need for BACK_BUFFER texture usage)
    BACK_BUFFER,

    /// @brief Specifies that this texture is a combined depth/stencil buffer.
    /// @note This may mean that this texture is completely GPU-sided and cannot be accessed by the CPU in any way.
    DEPTH_STENCIL_BUFFER,

    /// @brief Specifies that this texture isn't used for any special purpose.
    NORMAL,
};

class TextureResource : public RenderResource {
    friend RenderGraph;

private:
    const TextureUsage m_usage;
    VkFormat m_format{VK_FORMAT_UNDEFINED};

public:
    TextureResource(std::string &&name, TextureUsage usage) : RenderResource(name), m_usage(usage) {}

    /// @brief Specifies the format of this texture that is required when the physical texture is made.
    /// @details For TextureUsage::BACK_BUFFER textures, using the swapchain image format is preferable in most cases.
    /// For TextureUsage::DEPTH_STENCIL_BUFFER textures, a VK_FORMAT_D* must be used.
    void set_format(VkFormat format) {
        m_format = format;
    }
};

/// @brief A single render stage in the render graph.
/// @note Not to be confused with a vulkan render pass.
class RenderStage : public RenderGraphObject {
    friend RenderGraph;

private:
    const std::string m_name;
    std::unique_ptr<PhysicalStage> m_physical;
    std::vector<const RenderResource *> m_writes;
    std::vector<const RenderResource *> m_reads;

    std::vector<VkDescriptorSetLayout> m_descriptor_layouts;
    std::vector<VkPushConstantRange> m_push_constant_ranges;
    std::function<void(const PhysicalStage &, const wrapper::CommandBuffer &)> m_on_record{[](auto &, auto &) {}};

protected:
    explicit RenderStage(std::string name) : m_name(std::move(name)) {}

public:
    RenderStage(const RenderStage &) = delete;
    RenderStage(RenderStage &&) = delete;
    ~RenderStage() override = default;

    RenderStage &operator=(const RenderStage &) = delete;
    RenderStage &operator=(RenderStage &&) = delete;

    /// @brief Specifies that this stage writes to `resource`.
    void writes_to(const RenderResource *resource);

    /// @brief Specifies that this stage reads from `resource`.
    void reads_from(const RenderResource *resource);

    /// @brief Binds a descriptor set layout to this render stage.
    /// @note This function will be removed in the near future, as we are aiming for users of the API to not have to
    /// deal with descriptors at all.
    // TODO: Refactor descriptor management in the render graph
    void add_descriptor_layout(VkDescriptorSetLayout layout) {
        m_descriptor_layouts.push_back(layout);
    }

    /// @brief Add a push constant range to this render stage.
    /// @param range The push constant range
    void add_push_constant_range(VkPushConstantRange range) {
        m_push_constant_ranges.push_back(range);
    }

    /// @brief Specifies a function that will be called during command buffer recording for this stage
    /// @details This function can be used to specify other vulkan commands during command buffer recording. The most
    /// common use for this is for draw commands.
    void set_on_record(std::function<void(const PhysicalStage &, const wrapper::CommandBuffer &)> on_record) {
        m_on_record = std::move(on_record);
    }
};

class GraphicsStage : public RenderStage {
    friend RenderGraph;

private:
    bool m_clears_screen{false};
    bool m_depth_test{false};
    bool m_depth_write{false};
    VkPipelineColorBlendAttachmentState m_blend_attachment{};
    std::unordered_map<const BufferResource *, std::uint32_t> m_buffer_bindings;
    std::vector<VkPipelineShaderStageCreateInfo> m_shaders;

public:
    explicit GraphicsStage(std::string &&name) : RenderStage(name) {}
    GraphicsStage(const GraphicsStage &) = delete;
    GraphicsStage(GraphicsStage &&) = delete;
    ~GraphicsStage() override = default;

    GraphicsStage &operator=(const GraphicsStage &) = delete;
    GraphicsStage &operator=(GraphicsStage &&) = delete;

    /// @brief Specifies that this stage should clear the screen before rendering.
    void set_clears_screen(bool clears_screen) {
        m_clears_screen = clears_screen;
    }

    /// @brief Specifies the depth options for this stage.
    /// @param depth_test Whether depth testing should be performed
    /// @param depth_write Whether depth writing should be performed
    void set_depth_options(bool depth_test, bool depth_write) {
        m_depth_test = depth_test;
        m_depth_write = depth_write;
    }

    /// @brief Set the blend attachment for this stage.
    /// @param blend_attachment The blend attachment
    void set_blend_attachment(VkPipelineColorBlendAttachmentState blend_attachment) {
        m_blend_attachment = blend_attachment;
    }

    /// @brief Specifies that `buffer` should map to `binding` in the shaders of this stage.
    void bind_buffer(const BufferResource *buffer, std::uint32_t binding);

    /// @brief Specifies that `shader` should be used during the pipeline of this stage.
    /// @note Binding two shaders of same type (e.g. two vertex shaders) is undefined behaviour.
    void uses_shader(const wrapper::Shader &shader);
};

// TODO: Add wrapper::Allocation that can be made by doing `device->make<Allocation>(...)`.
class PhysicalResource : public RenderGraphObject {
    friend RenderGraph;

protected:
    // TODO: Add OOP device functions (see above todo) and only store a wrapper::Device here.
    const VmaAllocator m_allocator;
    const VkDevice m_device;
    VmaAllocation m_allocation{VK_NULL_HANDLE};

    PhysicalResource(VmaAllocator allocator, VkDevice device) : m_allocator(allocator), m_device(device) {}

public:
    PhysicalResource(const PhysicalResource &) = delete;
    PhysicalResource(PhysicalResource &&) = delete;
    ~PhysicalResource() override = default;

    PhysicalResource &operator=(const PhysicalResource &) = delete;
    PhysicalResource &operator=(PhysicalResource &&) = delete;
};

class PhysicalBuffer : public PhysicalResource {
    friend RenderGraph;

private:
    VmaAllocationInfo m_alloc_info{};
    VkBuffer m_buffer{VK_NULL_HANDLE};

public:
    PhysicalBuffer(VmaAllocator allocator, VkDevice device) : PhysicalResource(allocator, device) {}
    PhysicalBuffer(const PhysicalBuffer &) = delete;
    PhysicalBuffer(PhysicalBuffer &&) = delete;
    ~PhysicalBuffer() override;

    PhysicalBuffer &operator=(const PhysicalBuffer &) = delete;
    PhysicalBuffer &operator=(PhysicalBuffer &&) = delete;
};

class PhysicalImage : public PhysicalResource {
    friend RenderGraph;

private:
    VkImage m_image{VK_NULL_HANDLE};
    VkImageView m_image_view{VK_NULL_HANDLE};

public:
    PhysicalImage(VmaAllocator allocator, VkDevice device) : PhysicalResource(allocator, device) {}
    PhysicalImage(const PhysicalImage &) = delete;
    PhysicalImage(PhysicalImage &&) = delete;
    ~PhysicalImage() override;

    PhysicalImage &operator=(const PhysicalImage &) = delete;
    PhysicalImage &operator=(PhysicalImage &&) = delete;
};

class PhysicalBackBuffer : public PhysicalResource {
    friend RenderGraph;

private:
    const wrapper::Swapchain &m_swapchain;

public:
    PhysicalBackBuffer(VmaAllocator allocator, VkDevice device, const wrapper::Swapchain &swapchain)
        : PhysicalResource(allocator, device), m_swapchain(swapchain) {}
    PhysicalBackBuffer(const PhysicalBackBuffer &) = delete;
    PhysicalBackBuffer(PhysicalBackBuffer &&) = delete;
    ~PhysicalBackBuffer() override = default;

    PhysicalBackBuffer &operator=(const PhysicalBackBuffer &) = delete;
    PhysicalBackBuffer &operator=(PhysicalBackBuffer &&) = delete;
};

class PhysicalStage : public RenderGraphObject {
    friend RenderGraph;

private:
    std::vector<wrapper::CommandBuffer> m_command_buffers;
    const wrapper::Device &m_device;
    std::unique_ptr<wrapper::Semaphore> m_finished_semaphore;
    VkPipeline m_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout m_pipeline_layout{VK_NULL_HANDLE};

protected:
    [[nodiscard]] VkDevice device() const {
        return m_device.device();
    }

public:
    explicit PhysicalStage(const wrapper::Device &device) : m_device(device) {}
    PhysicalStage(const PhysicalStage &) = delete;
    PhysicalStage(PhysicalStage &&) = delete;
    ~PhysicalStage() override;

    PhysicalStage &operator=(const PhysicalStage &) = delete;
    PhysicalStage &operator=(PhysicalStage &&) = delete;

    /// @brief Retrieve the pipeline layout of this physical stage.
    // TODO: This can be removed once descriptors are properly implemented in the render graph.
    [[nodiscard]] VkPipelineLayout pipeline_layout() const {
        return m_pipeline_layout;
    }
};

class PhysicalGraphicsStage : public PhysicalStage {
    friend RenderGraph;

private:
    VkRenderPass m_render_pass{VK_NULL_HANDLE};
    std::vector<wrapper::Framebuffer> m_framebuffers;

public:
    explicit PhysicalGraphicsStage(const wrapper::Device &device) : PhysicalStage(device) {}
    PhysicalGraphicsStage(const PhysicalGraphicsStage &) = delete;
    PhysicalGraphicsStage(PhysicalGraphicsStage &&) = delete;
    ~PhysicalGraphicsStage() override;

    PhysicalGraphicsStage &operator=(const PhysicalGraphicsStage &) = delete;
    PhysicalGraphicsStage &operator=(PhysicalGraphicsStage &&) = delete;
};

class RenderGraph {
private:
    const wrapper::Device &m_device;
    VkCommandPool m_command_pool{VK_NULL_HANDLE};
    const wrapper::Swapchain &m_swapchain;
    std::shared_ptr<spdlog::logger> m_log{spdlog::default_logger()->clone("render-graph")};

    // Vectors of render resources and stages.
    std::vector<std::unique_ptr<BufferResource>> m_buffer_resources;
    std::vector<std::unique_ptr<TextureResource>> m_texture_resources;
    std::vector<std::unique_ptr<RenderStage>> m_stages;

    // Stage execution order.
    std::vector<RenderStage *> m_stage_stack;

    // Functions for building resource related vulkan objects.
    void build_buffer(const BufferResource &, PhysicalBuffer &) const;
    void build_image(const TextureResource &, PhysicalImage &, VmaAllocationCreateInfo *) const;
    void build_image_view(const TextureResource &, PhysicalImage &) const;

    // Functions for building stage related vulkan objects.
    void build_pipeline_layout(const RenderStage *, PhysicalStage &) const;
    void record_command_buffer(const RenderStage *, PhysicalStage &, std::uint32_t image_index) const;

    // Functions for building graphics stage related vulkan objects.
    void build_render_pass(const GraphicsStage *, PhysicalGraphicsStage &) const;
    void build_graphics_pipeline(const GraphicsStage *, PhysicalGraphicsStage &) const;

public:
    RenderGraph(const wrapper::Device &device, VkCommandPool command_pool, const wrapper::Swapchain &swapchain)
        : m_device(device), m_command_pool(command_pool), m_swapchain(swapchain) {}

    /// @brief Adds either a render resource or render stage to the render graph.
    /// @return A mutable reference to the just-added resource or stage
    template <typename T, typename... Args>
    T *add(Args &&...args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        if constexpr (std::is_same_v<T, BufferResource>) {
            return static_cast<T *>(m_buffer_resources.emplace_back(std::move(ptr)).get());
        } else if constexpr (std::is_same_v<T, TextureResource>) {
            return static_cast<T *>(m_texture_resources.emplace_back(std::move(ptr)).get());
        } else if constexpr (std::is_base_of_v<RenderStage, T>) {
            return static_cast<T *>(m_stages.emplace_back(std::move(ptr)).get());
        } else {
            static_assert(!std::is_same_v<T, T>, "T must be a RenderResource or RenderStage");
        }
    }

    /// @brief Compiles the render graph resources/stages into physical vulkan objects.
    /// @param target The target resource of the render graph (usually the back buffer)
    void compile(const RenderResource *target);

    /// @brief Submits the command frame's command buffers for drawing.
    /// @param image_index The current image index, retrieved from Swapchain::acquire_next_image
    /// @param wait_semaphore The semaphore to wait on before rendering, typically some kind of "swapchain image
    /// available" semaphore
    /// @param graphics_queue The graphics queue to push rendering commands to
    VkSemaphore render(std::uint32_t image_index, VkSemaphore wait_semaphore, VkQueue graphics_queue);
};

template <typename T>
[[nodiscard]] T *RenderGraphObject::as() {
    return dynamic_cast<T *>(this);
}

template <typename T>
[[nodiscard]] const T *RenderGraphObject::as() const {
    return dynamic_cast<const T *>(this);
}

template <typename T>
void BufferResource::upload_data(const T *data, std::size_t count) {
    m_data = data;
    m_data_size = count * (m_element_size = sizeof(T));
    m_data_upload_needed = true;
}

template <typename T>
void BufferResource::upload_data(const std::vector<T> &data) {
    upload_data(data.data(), data.size());
}

} // namespace inexor::vulkan_renderer
