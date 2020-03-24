#include "vk_uniform_buffer_manager.hpp"


namespace inexor {
namespace vulkan_renderer {


	VulkanUniformBufferManager::VulkanUniformBufferManager()
	{
	}

	
	VulkanUniformBufferManager::~VulkanUniformBufferManager()
	{
	}


	VkResult VulkanUniformBufferManager::initialise(const VkDevice& device, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager, VmaAllocator& vma_allocator)
	{
		assert(device);
		assert(debug_marker_manager);
		assert(vma_allocator);

		spdlog::debug("Initialising uniorm buffer manager.");

		this->device = device;
		this->debug_marker_manager = debug_marker_manager;
		this->vma_allocator = vma_allocator;

		spdlog::debug("Clearing uniform buffer storage.");

		uniform_buffer_initialised = true;

		return VK_SUCCESS;
	}


	VkResult VulkanUniformBufferManager::create_buffer(std::string& internal_buffer_name, InexorBuffer& buffer_object, const VkDeviceSize& buffer_size)
	{
		assert(uniform_buffer_initialised);
		assert(vma_allocator);
		assert(debug_marker_manager);
		assert(buffer_size>0);
		
		spdlog::debug("Allocating memory for uniform buffer '{}'.", internal_buffer_name);

		buffer_object.create_info.sType                = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_object.create_info.size                 = buffer_size;
		buffer_object.create_info.usage                = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_object.create_info.sharingMode          = VK_SHARING_MODE_EXCLUSIVE;
		buffer_object.allocation_create_info.usage     = VMA_MEMORY_USAGE_CPU_TO_GPU;
		buffer_object.allocation_create_info.flags     = VMA_ALLOCATION_CREATE_MAPPED_BIT|VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
		buffer_object.allocation_create_info.pUserData = internal_buffer_name.data();

		// Allocate memory for the uniform buffer.
		VkResult result = vmaCreateBuffer(vma_allocator, &buffer_object.create_info, &buffer_object.allocation_create_info, &buffer_object.buffer, &buffer_object.allocation, &buffer_object.allocation_info);
		vulkan_error_check(result);

		return result;
	}


	VkResult VulkanUniformBufferManager::create_uniform_buffer(const std::string& internal_uniform_buffer_name, const VkDeviceSize& uniform_buffer_size, const std::size_t number_of_images_in_swapchain)
	{
		assert(uniform_buffer_initialised);
		assert(uniform_buffer_size>0);
		assert(internal_uniform_buffer_name.length()>0);
		assert(number_of_images_in_swapchain>0);

		if(does_key_exist(internal_uniform_buffer_name))
		{
			spdlog::error("A uniform buffer with the name '{}' does already exist!");
			return VK_ERROR_INITIALIZATION_FAILED;
		}

		spdlog::debug("Creating uniform buffer '{}' for {} images in swapchain.", internal_uniform_buffer_name, number_of_images_in_swapchain);
		
		std::shared_ptr<InexorUniformBuffer> new_uniform_buffer = std::make_shared<InexorUniformBuffer>();

		new_uniform_buffer->setup(number_of_images_in_swapchain);


		// Create one uniform buffer for every image in the swapchain.
		for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			spdlog::debug("Creating uniform buffer #{}", i);

			// Automatically iterate the naming of the uniform buffers.
			std::string uniform_buffer_description = "Uniform buffer '"+ internal_uniform_buffer_name +"' #"+ std::to_string(i);

			// Create the new uniform buffer.
			VkResult result = create_buffer(uniform_buffer_description, new_uniform_buffer->buffers[i], uniform_buffer_size);
			vulkan_error_check(result);

			// Give this uniform buffer an appropriate name.
			debug_marker_manager->set_object_name(device, (uint64_t)(&new_uniform_buffer->buffers[i]), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, uniform_buffer_description.c_str());
		}

		// Store the new uniform buffer in the map.
		// Call base class method.
		add_entry(internal_uniform_buffer_name, new_uniform_buffer);

		return VK_SUCCESS;
	}

	
	std::optional<std::shared_ptr<InexorUniformBuffer>> VulkanUniformBufferManager::get_uniform_buffer(const std::string& uniform_buffer_name)
	{
		assert(uniform_buffer_initialised);

		if(!does_key_exist(uniform_buffer_name))
		{
			return std::nullopt;
		}

		// Get the uniform buffer by internal name (key).
		std::optional<std::shared_ptr<InexorUniformBuffer>> return_value = get_entry(uniform_buffer_name);

		return return_value;
	}


	VkResult VulkanUniformBufferManager::update_uniform_buffer(const std::string& internal_uniform_buffer_name, const std::size_t current_image_index, void* data_source_address, const std::size_t uniform_buffer_size)
	{
		assert(uniform_buffer_initialised);
		assert(internal_uniform_buffer_name.length()>0);
		assert(data_source_address);
		assert(uniform_buffer_size>0);
		
		// Call base class method.
		if(!does_key_exist(internal_uniform_buffer_name))
		{
			spdlog::error("Uniform buffer '{}' does not exist!", internal_uniform_buffer_name);
			spdlog::error("Uniform buffer manager does not create buffers automatically when calling update method!");
			return VK_ERROR_INITIALIZATION_FAILED;
		}

		auto uniform_buffer = get_entry(internal_uniform_buffer_name);

		if(!uniform_buffer.has_value())
		{
			spdlog::error("Uniform buffer '{}' does not exist!", internal_uniform_buffer_name);
			return VK_ERROR_INITIALIZATION_FAILED;
		}
		
		// Use a lock guard to ensure thread-safety.
		std::lock_guard<std::mutex> lock(uniform_buffer_manager_mutex);

		// Update the uniform buffer.
		uniform_buffer.value()->update_buffers(data_source_address, uniform_buffer_size);

		return VK_SUCCESS;
	}


	VkResult VulkanUniformBufferManager::shutdown_uniform_buffers()
	{
		assert(uniform_buffer_initialised);
		assert(vma_allocator);

		spdlog::debug("Destroying uniform buffers.");

		auto all_uniform_buffers = get_all_values();

		// Use a lock guard to ensure thread-safety.
		std::lock_guard<std::mutex> lock(uniform_buffer_manager_mutex);

		for(const auto& uniform_buffer : all_uniform_buffers)
		{
			for(auto& buffer : uniform_buffer->buffers)
			{
				vmaDestroyBuffer(vma_allocator, buffer.buffer, buffer.allocation);
				buffer.buffer = VK_NULL_HANDLE;
			}
			
			uniform_buffer->shutdown();
		}

		// Call base class method.
		delete_all_entries();
		
		return VK_SUCCESS;
	}

};
};
