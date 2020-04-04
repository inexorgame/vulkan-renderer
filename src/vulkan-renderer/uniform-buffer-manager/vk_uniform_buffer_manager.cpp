#include "vk_uniform_buffer_manager.hpp"


namespace inexor {
namespace vulkan_renderer {


	VkResult VulkanUniformBufferManager::initialise(const VkDevice& device,
	                                                const VmaAllocator& vma_allocator,
													const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager)
	{
		assert(device);
		assert(vma_allocator);
		assert(debug_marker_manager);

		spdlog::debug("Initialising uniorm buffer manager.");

		this->device = device;
		this->debug_marker_manager = debug_marker_manager;
		this->vma_allocator = vma_allocator;
		this->number_of_images_in_swapchain = number_of_images_in_swapchain;

		spdlog::debug("Clearing uniform buffer storage.");

		uniform_buffer_initialised = true;

		return VK_SUCCESS;
	}


	VkResult VulkanUniformBufferManager::create_buffer(std::string& internal_buffer_name,
	                                                   const VkDeviceSize& buffer_size,
													   std::shared_ptr<InexorUniformBuffer>& buffer_object)
	{
		assert(uniform_buffer_initialised);
		assert(vma_allocator);
		assert(debug_marker_manager);
		assert(buffer_size>0);
		
		spdlog::debug("Allocating memory for uniform buffer '{}'.", internal_buffer_name);

		buffer_object->create_info.sType                = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_object->create_info.size                 = buffer_size;
		buffer_object->create_info.usage                = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_object->create_info.sharingMode          = VK_SHARING_MODE_EXCLUSIVE;
		buffer_object->allocation_create_info.usage     = VMA_MEMORY_USAGE_CPU_TO_GPU;
		buffer_object->allocation_create_info.flags     = VMA_ALLOCATION_CREATE_MAPPED_BIT|VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
		buffer_object->allocation_create_info.pUserData = internal_buffer_name.data();

		// Allocate memory for the uniform buffer.
		VkResult result = vmaCreateBuffer(vma_allocator, &buffer_object->create_info, &buffer_object->allocation_create_info, &buffer_object->buffer, &buffer_object->allocation, &buffer_object->allocation_info);
		vulkan_error_check(result);

		return result;
	}


	VkResult VulkanUniformBufferManager::create_uniform_buffer(const std::string& internal_uniform_buffer_name,
	                                                           const VkDeviceSize& uniform_buffer_size,
															   std::shared_ptr<InexorUniformBuffer>& uniform_buffer)
	{
		assert(uniform_buffer_initialised);
		assert(uniform_buffer_size>0);
		assert(!internal_uniform_buffer_name.empty());

		if(does_key_exist(internal_uniform_buffer_name))
		{
			spdlog::error("A uniform buffer with the name '{}' does already exist!");
			return VK_ERROR_INITIALIZATION_FAILED;
		}

		spdlog::debug("Creating uniform buffer '{}'", internal_uniform_buffer_name);

		uniform_buffer = std::make_shared<InexorUniformBuffer>();

		// Automatically iterate the naming of the uniform buffers.
		std::string uniform_buffer_description = "Uniform buffer '"+ internal_uniform_buffer_name +".";

		// Create the new uniform buffer.
		VkResult result = create_buffer(uniform_buffer_description, uniform_buffer_size, uniform_buffer);
		vulkan_error_check(result);

		// Give this uniform buffer an appropriate name using a Vulkan debug marker.
		// TODO: FIX!
		//debug_marker_manager->set_object_name(device, (uint64_t)(&uniform_buffer->buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, uniform_buffer_description.c_str());

		// Store the new uniform buffer in the map.
		// Call base class method.
		add_entry(internal_uniform_buffer_name, uniform_buffer);

		return VK_SUCCESS;
	}


	// TODO: Implement update_uniform_buffer() by target memory address. Maybe do not check if value exists in unordered_map for performance reasons? Use shared_mutex!
	VkResult VulkanUniformBufferManager::update_uniform_buffer(const std::string& internal_uniform_buffer_name,
	                                                           void* data_source_address,
															   const std::size_t uniform_buffer_size)
	{
		assert(uniform_buffer_initialised);
		assert(!internal_uniform_buffer_name.empty());
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
		
		// Lock write access.
		std::unique_lock<std::shared_mutex> lock(type_manager_shared_mutex);

		// Update the uniform buffer memory!
		std::memcpy(uniform_buffer.value()->allocation_info.pMappedData, data_source_address, uniform_buffer_size);

		return VK_SUCCESS;
	}


	VkResult VulkanUniformBufferManager::destroy_uniform_buffers()
	{
		auto all_uniform_buffers = get_all_values();
		
		// Lock write access.
		std::unique_lock<std::shared_mutex> lock(type_manager_shared_mutex);

		for(const auto& uniform_buffer : all_uniform_buffers)
		{
			vmaDestroyBuffer(vma_allocator, uniform_buffer->buffer, uniform_buffer->allocation);
			uniform_buffer->buffer = VK_NULL_HANDLE;
		}

		return VK_SUCCESS;
	}


	VkResult VulkanUniformBufferManager::shutdown_uniform_buffers()
	{
		assert(uniform_buffer_initialised);
		assert(vma_allocator);

		spdlog::debug("Destroying uniform buffers.");

		destroy_uniform_buffers();

		// Call base class method.
		delete_all_entries();
		
		return VK_SUCCESS;
	}

};
};
