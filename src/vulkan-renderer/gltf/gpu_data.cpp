#include "inexor/vulkan-renderer/gltf/gpu_data.hpp"

#include "inexor/vulkan-renderer/exception.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::gltf {

ModelGpuPbrData::ModelGpuPbrData(RenderGraph *render_graph, const ModelCpuData &model_cpu_data,
                                 const wrapper::UniformBuffer<DefaultUBO> &shader_data_model,
                                 const wrapper::UniformBuffer<pbr::ModelPbrShaderParamsUBO> &shader_data_pbr,
                                 VkDescriptorImageInfo brdf_lut_texture)
    : m_device(render_graph->device_wrapper()), m_brdf_lut_texture(brdf_lut_texture),
      ModelGpuPbrDataBase(render_graph->device_wrapper(), model_cpu_data.model()) {

    m_shader_params = std::make_unique<wrapper::UniformBuffer<pbr::ModelPbrShaderParamsUBO>>(
        render_graph->device_wrapper(), "skybox");

    m_shader_values_scene =
        std::make_unique<wrapper::UniformBuffer<DefaultUBO>>(render_graph->device_wrapper(), "skybox");

    load_textures();
    load_materials();
    load_nodes();
    load_animations();
    load_skins();
    setup_rendering_resources(render_graph);
}

ModelGpuPbrData::ModelGpuPbrData(ModelGpuPbrData &&other) noexcept
    : m_device(other.m_device), ModelGpuPbrDataBase(std::move(other)) {
    m_name = std::move(other.m_name);
    m_model_scale = other.m_model_scale;
    m_scene_descriptor_set = other.m_scene_descriptor_set;
    m_material_descriptor_set = other.m_material_descriptor_set;
    m_brdf_lut_texture = other.m_brdf_lut_texture;
    m_enviroment_cube_texture = other.m_enviroment_cube_texture;
    m_irradiance_cube_texture = other.m_irradiance_cube_texture;
    m_prefiltered_cube_texture = other.m_prefiltered_cube_texture;
    m_node_descriptor_set_layout = other.m_node_descriptor_set_layout;
    m_material_descriptor_set_layout = other.m_material_descriptor_set_layout;
    m_scene_descriptor_set_layout = other.m_scene_descriptor_set_layout;
    m_shader_params = std::exchange(other.m_shader_params, nullptr);
    m_shader_values_scene = std::exchange(other.m_shader_values_scene, nullptr);
}

ModelGpuPbrData::~ModelGpuPbrData() {
    vkDestroyDescriptorSetLayout(m_device.device(), m_scene_descriptor_set_layout, nullptr);
    vkDestroyDescriptorSetLayout(m_device.device(), m_material_descriptor_set_layout, nullptr);
    vkDestroyDescriptorSetLayout(m_device.device(), m_node_descriptor_set_layout, nullptr);
}

void ModelGpuPbrData::setup_node_descriptor_sets(const VkDevice device, const ModelNode &node) {
    if (node.mesh) {
        // TODO: Use descriptor builder!
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool = m_descriptor_pool;
        descriptorSetAllocInfo.pSetLayouts = &m_node_descriptor_set_layout;
        descriptorSetAllocInfo.descriptorSetCount = 1;

        vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &node.mesh->descriptor_set);

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.dstSet = node.mesh->descriptor_set;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.pBufferInfo = &node.mesh->uniform_buffer->descriptor_buffer_info;

        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
    }

    for (const auto &child : node.children) {
        setup_node_descriptor_sets(device, child);
    }
}

void ModelGpuPbrData::setup_rendering_resources(RenderGraph *render_graph) {
    std::uint32_t image_sampler_count = 0;
    std::uint32_t material_count = 0;
    std::uint32_t mesh_count = 0;

    // Environment samplers (radiance, irradiance, brdf lookup table)
    image_sampler_count += 3;

    for (const auto &material : materials()) {
        image_sampler_count += 5;
        material_count++;
    }
    for (const auto &node : linear_nodes()) {
        if (node.mesh) {
            mesh_count++;
        }
    }

    const std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (4 + mesh_count)},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, image_sampler_count}};

    VkDescriptorPoolCreateInfo descriptorPoolCI{};
    descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCI.poolSizeCount = 2;
    descriptorPoolCI.pPoolSizes = poolSizes.data();
    descriptorPoolCI.maxSets = (2 + material_count + mesh_count);

    // TODO: Error checks!
    vkCreateDescriptorPool(render_graph->device(), &descriptorPoolCI, nullptr, &m_descriptor_pool);

    // Scene (matrices and environment maps)
    {
        std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
             nullptr},
            {1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        };

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
        descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCI.pBindings = set_layout_bindings.data();
        descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());

        vkCreateDescriptorSetLayout(render_graph->device(), &descriptorSetLayoutCI, nullptr,
                                    &m_scene_descriptor_set_layout);

        VkDescriptorSetAllocateInfo desc_set_ai{};
        desc_set_ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        desc_set_ai.descriptorPool = m_descriptor_pool;
        desc_set_ai.pSetLayouts = &m_scene_descriptor_set_layout;
        desc_set_ai.descriptorSetCount = 1;

        vkAllocateDescriptorSets(render_graph->device(), &desc_set_ai, &m_scene_descriptor_set);

        std::array<VkWriteDescriptorSet, 5> write_desc_sets{};

        write_desc_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_desc_sets[0].descriptorCount = 1;
        write_desc_sets[0].dstSet = m_scene_descriptor_set;
        write_desc_sets[0].dstBinding = 0;
        write_desc_sets[0].pBufferInfo = &m_shader_values_scene->descriptor_buffer_info;

        write_desc_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_desc_sets[1].descriptorCount = 1;
        write_desc_sets[1].dstSet = m_scene_descriptor_set;
        write_desc_sets[1].dstBinding = 1;
        write_desc_sets[1].pBufferInfo = &m_shader_params->descriptor_buffer_info;

        write_desc_sets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc_sets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_desc_sets[2].descriptorCount = 1;
        write_desc_sets[2].dstSet = m_scene_descriptor_set;
        write_desc_sets[2].dstBinding = 2;
        write_desc_sets[2].pImageInfo = &m_irradiance_cube_texture;

        write_desc_sets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc_sets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_desc_sets[3].descriptorCount = 1;
        write_desc_sets[3].dstSet = m_scene_descriptor_set;
        write_desc_sets[3].dstBinding = 3;
        write_desc_sets[3].pImageInfo = &m_prefiltered_cube_texture;

        write_desc_sets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc_sets[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_desc_sets[4].descriptorCount = 1;
        write_desc_sets[4].dstSet = m_scene_descriptor_set;
        write_desc_sets[4].dstBinding = 4;
        write_desc_sets[4].pImageInfo = &m_brdf_lut_texture;

        vkUpdateDescriptorSets(m_device.device(), static_cast<uint32_t>(write_desc_sets.size()), write_desc_sets.data(),
                               0, nullptr);
    }

    // Material (samplers)
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
            {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
            {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
        };

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
        descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCI.pBindings = setLayoutBindings.data();
        descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

        vkCreateDescriptorSetLayout(render_graph->device(), &descriptorSetLayoutCI, nullptr,
                                    &m_material_descriptor_set_layout);

        for (const auto &material : materials()) {

            VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
            descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorSetAllocInfo.descriptorPool = m_descriptor_pool;
            descriptorSetAllocInfo.pSetLayouts = &m_material_descriptor_set_layout;
            descriptorSetAllocInfo.descriptorSetCount = 1;

            vkAllocateDescriptorSets(render_graph->device(), &descriptorSetAllocInfo, &m_material_descriptor_set);

            std::vector<VkDescriptorImageInfo> imageDescriptors = {
                m_empty_texture->descriptor_image_info(), m_empty_texture->descriptor_image_info(),
                material.normal_texture ? material.normal_texture->descriptor_image_info()
                                        : m_empty_texture->descriptor_image_info(),
                material.occlusion_texture ? material.occlusion_texture->descriptor_image_info()
                                           : m_empty_texture->descriptor_image_info(),
                material.emissive_texture ? material.emissive_texture->descriptor_image_info()
                                          : m_empty_texture->descriptor_image_info()};

            // TODO: glTF specs states that metallic roughness should be preferred, even if specular glosiness is
            // present

            if (material.metallic_roughness) {
                if (material.base_color_texture) {
                    imageDescriptors[0] = material.base_color_texture->descriptor_image_info();
                }
                if (material.metallic_roughness_texture) {
                    imageDescriptors[1] = material.metallic_roughness_texture->descriptor_image_info();
                }
            }

            if (material.specular_glossiness) {
                if (material.extension.diffuse_texture) {
                    imageDescriptors[0] = material.extension.diffuse_texture->descriptor_image_info();
                }
                if (material.extension.specular_glossiness_texture) {
                    imageDescriptors[1] = material.extension.specular_glossiness_texture->descriptor_image_info();
                }
            }

            std::array<VkWriteDescriptorSet, 5> write_desc_set{};

            for (size_t i = 0; i < imageDescriptors.size(); i++) {
                write_desc_set[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write_desc_set[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                write_desc_set[i].descriptorCount = 1;
                write_desc_set[i].dstSet = m_material_descriptor_set;
                write_desc_set[i].dstBinding = static_cast<uint32_t>(i);
                write_desc_set[i].pImageInfo = &imageDescriptors[i];
            }

            vkUpdateDescriptorSets(m_device.device(), static_cast<uint32_t>(write_desc_set.size()),
                                   write_desc_set.data(), 0, nullptr);
        }

        // Model node (matrices)
        std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
        };

        // TODO: Move into make_info<> template!
        VkDescriptorSetLayoutCreateInfo desc_set_layout_ci{};
        desc_set_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        desc_set_layout_ci.pBindings = setLayoutBindings.data();
        desc_set_layout_ci.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());

        if (const auto result = vkCreateDescriptorSetLayout(render_graph->device(), &desc_set_layout_ci, nullptr,
                                                            &m_node_descriptor_set_layout);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vkCreateDescriptorSetLayout failed!", result);
        }

        for (const auto &node : nodes()) {
            setup_node_descriptor_sets(render_graph->device(), node);
        }
    }
}

} // namespace inexor::vulkan_renderer::gltf
