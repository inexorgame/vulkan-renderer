#include "shader.hpp"


namespace inexor {
namespace vulkan_renderer {


	InexorShader::InexorShader()
	{
	}
	

	InexorShader::~InexorShader()
	{
	}


	void InexorShader::set_shader_name(const std::string& shader_name)
	{
		assert(shader_name.length()>0);
		name = shader_name;
	}

	
	void InexorShader::set_shader_entry_point(const std::string& entry_point_name)
	{
		assert(entry_point_name.length()>0);
		entry_name = entry_point_name;
	}


	void InexorShader::set_shader_type(const VkShaderStageFlagBits& shader_type)
	{
		type = shader_type;
	}

	
	void InexorShader::set_shader_module(const VkShaderModule& shader_module)
	{
		vulkan_shader_module = shader_module;
	}


	const std::string InexorShader::get_shader_name() const
	{
		return name;
	}


	const std::string InexorShader::get_shader_entry_point() const
	{
		return entry_name;
	}


	const VkShaderStageFlagBits InexorShader::get_shader_type() const
	{
		return type;
	}
			

	const VkShaderModule InexorShader::get_shader_module() const
	{
		return vulkan_shader_module;
	}


};
};
