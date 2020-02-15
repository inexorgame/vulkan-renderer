#include "InexorVulkanShader.hpp"


namespace inexor {
namespace vulkan_renderer {


	InexorVulkanShader::InexorVulkanShader()
	{
	}
	

	InexorVulkanShader::~InexorVulkanShader()
	{
	}


	void InexorVulkanShader::set_shader_name(const std::string& shader_name)
	{
		name = shader_name;
	}

	
	void InexorVulkanShader::set_shader_entry_point(const std::string& entry_point_name)
	{
		entry_name = entry_point_name;
	}


	void InexorVulkanShader::set_shader_type(const VkShaderStageFlagBits& shader_type)
	{
		type = shader_type;
	}

	
	void InexorVulkanShader::set_shader_module(const VkShaderModule& shader_module)
	{
		vulkan_shader_module = shader_module;
	}


	const std::string InexorVulkanShader::get_shader_name() const
	{
		return name;
	}


	const std::string InexorVulkanShader::get_shader_entry_point() const
	{
		return entry_name;
	}


	const VkShaderStageFlagBits InexorVulkanShader::get_shader_type() const
	{
		return type;
	}
			

	const VkShaderModule InexorVulkanShader::get_shader_module() const
	{
		return vulkan_shader_module;
	}


};
};
