#include "VulkanShaderManager.hpp"
using namespace std;


namespace inexor {
namespace vulkan_renderer {

	
	VulkanShaderManager::VulkanShaderManager()
	{
	}


	VulkanShaderManager::~VulkanShaderManager()
	{
	}

	
	void VulkanShaderManager::initialise(const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager_instance)
	{
		assert(debug_marker_manager_instance);

		spdlog::debug("Initialising Vulkan shader manager.");
		dbg_marker_manager = debug_marker_manager_instance;
	}

	
	VkResult VulkanShaderManager::create_shader_module(const VkDevice& vulkan_device, const std::vector<char>& SPIRV_shader_bytes, VkShaderModule* shader_module)
	{
		assert(vulkan_device);
		assert(SPIRV_shader_bytes.size()>0);
		assert(dbg_marker_manager);
		
		spdlog::debug("SPIR-V shader byte size: {}.", SPIRV_shader_bytes.size());

		VkShaderModuleCreateInfo shader_create_info = {};
	
		shader_create_info.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_create_info.pNext    = nullptr;
		shader_create_info.flags    = 0;
		shader_create_info.codeSize = SPIRV_shader_bytes.size();

		// When you perform a cast like this, you also need to ensure that the data satisfies the alignment requirements of uint32_t.
		// Lucky for us, the data is stored in an std::vector where the default allocator already ensures that the data satisfies the worst case alignment requirements.
		shader_create_info.pCode    = reinterpret_cast<const uint32_t*>(SPIRV_shader_bytes.data());

		return vkCreateShaderModule(vulkan_device, &shader_create_info, nullptr, shader_module);
	}


	VkResult VulkanShaderManager::create_shader_from_byte_buffer(const VkDevice& vulkan_device, const VkShaderStageFlagBits& shader_type, const std::vector<char>& SPIRV_shader_bytes, const std::string& shader_name, const std::string& shader_entry_point)
	{
		assert(vulkan_device);
		assert(shader_name.length()>0);
		assert(SPIRV_shader_bytes.size()>0);
		assert(dbg_marker_manager);

		spdlog::debug("Creating shader {} from byte buffer.", shader_name.c_str());
		
		InexorVulkanShader new_shader;

		new_shader.set_shader_type(shader_type);
		new_shader.set_shader_name(shader_name);
		new_shader.set_shader_entry_point(shader_entry_point);

		// Create the shader module from the SPIR-V byte buffer.
		VkShaderModule shader_module;
		VkResult result = create_shader_module(vulkan_device, SPIRV_shader_bytes, &shader_module);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}
		
		// Store the generated shader module.
		new_shader.set_shader_module(shader_module);

		// Add this shader to the list.
		shaders.push_back(new_shader);
		
		return VK_SUCCESS;
	}


	VkResult VulkanShaderManager::create_shader_from_file(const VkDevice& vulkan_device, const VkShaderStageFlagBits& shader_type, const std::string& SPIRV_shader_file_name, const std::string& shader_name, const std::string& shader_entry_point)
	{
		assert(vulkan_device);
		assert(SPIRV_shader_file_name.length()>0);
		assert(dbg_marker_manager);
		
		spdlog::debug("Creating shader {} from file.", SPIRV_shader_file_name.c_str());

		InexorVulkanShader new_fragment_shader;

		// Load the fragment shader into memory.
		new_fragment_shader.load_file(SPIRV_shader_file_name);

		new_fragment_shader.set_shader_entry_point(shader_entry_point);
		new_fragment_shader.set_shader_name(shader_name);

		//This is a fragment shader.
		new_fragment_shader.set_shader_type(shader_type);

		VkShaderModule new_shader_module;
		
		// Create a Vulkan shader module.
		VkResult result = create_shader_module(vulkan_device, new_fragment_shader.get_file_data(), &new_shader_module);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		std::string internal_shader_name = "Shader module '"+ SPIRV_shader_file_name +"'.";

		// Give this shader an appropriate name.
		dbg_marker_manager->set_object_name(vulkan_device, (uint64_t)(new_shader_module), VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, internal_shader_name.c_str());

		// Store the generated shader in memory.
		new_fragment_shader.set_shader_module(new_shader_module);

		// Add this shader to the list of shaders.
		shaders.push_back(new_fragment_shader);
		
		return VK_SUCCESS;
	}


	void VulkanShaderManager::shutdown_shaders(const VkDevice& vulkan_device)
	{
		assert(vulkan_device);
		assert(dbg_marker_manager);

		spdlog::debug("Shutting down shader manager.");

		for(std::size_t i=0; i<shaders.size(); i++)
		{
			spdlog::debug("Destroying shader module {}.", shaders[i].get_shader_name());
			
			vkDestroyShaderModule(vulkan_device, shaders[i].get_shader_module(), nullptr);
		}

		shaders.clear();
	}


	const std::vector<InexorVulkanShader> VulkanShaderManager::get_shaders() const
	{
		return shaders;
	}


};
};
