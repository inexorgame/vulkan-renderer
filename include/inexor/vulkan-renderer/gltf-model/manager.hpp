#pragma once

#include "inexor/vulkan-renderer/descriptor_manager.hpp"
#include "inexor/vulkan-renderer/gltf-model/animation.hpp"
#include "inexor/vulkan-renderer/gltf-model/animation_channel.hpp"
#include "inexor/vulkan-renderer/gltf-model/animation_sampler.hpp"
#include "inexor/vulkan-renderer/gltf-model/bounding_box.hpp"
#include "inexor/vulkan-renderer/gltf-model/dimensions.hpp"
#include "inexor/vulkan-renderer/gltf-model/material.hpp"
#include "inexor/vulkan-renderer/gltf-model/mesh.hpp"
#include "inexor/vulkan-renderer/gltf-model/model.hpp"
#include "inexor/vulkan-renderer/gltf-model/primitive.hpp"
#include "inexor/vulkan-renderer/gltf-model/skin.hpp"
#include "inexor/vulkan-renderer/gltf-model/texture_sampler.hpp"
#include "inexor/vulkan-renderer/gltf-model/vertex.hpp"
#include "inexor/vulkan-renderer/manager_template.hpp"
#include "inexor/vulkan-renderer/mesh_buffer_manager.hpp"
#include "inexor/vulkan-renderer/texture_manager.hpp"
#include "inexor/vulkan-renderer/uniform_buffer_manager.hpp"

#include <nlohmann/json.hpp>
#define TINYGLTF_NO_INCLUDE_JSON
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include <fstream>
#include <memory>
#include <stdlib.h>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::gltf_model {

/// TODO: Make ModelLoader and inherit!
/// @brief A manager class for model in glTF 2.0 format.
/// https://www.khronos.org/gltf/
struct Manager : ManagerClassTemplate<Model> {

public:
    Manager() = default;

    ~Manager() = default;

private:
    VkDevice device;

    bool model_manager_initialised = false;

    std::shared_ptr<VulkanTextureManager> texture_manager;

    std::shared_ptr<UniformBufferManager> uniform_buffer_manager;

    std::shared_ptr<MeshBufferManager> mesh_buffer_manager;

    std::shared_ptr<DescriptorManager> descriptor_manager;

    // The global descriptor bundle for glTF 2.0 models.
    std::shared_ptr<DescriptorBundle> gltf_global_descriptor_bundle;

public:
    /// @brief Initialises Vulkan glTF 2.0 model manager.
    /// @param device [in] The Vulkan device.
    /// @param global_descriptor_bundle [in] The global descriptor bundle.
    /// @param texture_manager [in] A shared pointer to the texture manager.
    /// @param uniform_buffer_manager [in] A shared pointer to the uniform buffer manager.
    /// @param mesh_buffer_manager [in] mesh_buffer_manager A shared pointer to the mesh buffer manager.
    VkResult init(const VkDevice &device, const std::shared_ptr<VulkanTextureManager> texture_manager,
                  const std::shared_ptr<UniformBufferManager> uniform_buffer_manager, const std::shared_ptr<MeshBufferManager> mesh_buffer_manager,
                  const std::shared_ptr<DescriptorManager> descriptor_manager);

    /// @brief Loads a glTF 2.0 file.
    /// @param internal_model_name [in] The internal name of the glTF 2.0 model which is used inside of the engine.
    /// @param gltf2_file_name [in] The filename of the glTF 2.0 file.
    VkResult load_model_from_glTF2_file(const std::string &internal_model_name, const std::string &gltf2_file_name);

    /// @brief Renders a certain model during the recording of a command buffer.
    /// @param internal_model_name [in] The internal name of the glTF 2.0 model.
    /// The model must be loaded at this point already.
    /// @param command_buffer[in] The command buffer which is being recorded.
    /// @param pipeline_layout [in] The pipeline layout.
    /// @param current_image_index [in] The current frame index.
    VkResult render_model(const std::string &internal_model_name, VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout,
                          std::size_t current_image_index);

    /// @brief Renders all models during the recording of a command buffer.
    /// @param command_buffer[in] The command buffer which is being recorded.
    /// @param pipeline_layout [in] The pipeline layout.
    /// @param current_image_index [in] The current frame index.
    VkResult render_all_models(VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, std::size_t current_image_index);

    VkResult create_model_descriptors(const std::size_t number_of_images_in_swapchain);

    /// @brief Returns the number of existing models.
    std::size_t get_model_count();

private:
    /// @brief Sets up descriptor sets for glTF model nodes.
    /// @param node [in] A glTF model node.
    VkResult setup_node_descriptor_set(std::shared_ptr<ModelNode> node);

    VkResult load_model_from_file(const std::string &file_name, std::shared_ptr<Model> &new_model, const float scale = 1.0f);

    void destroy();

    void load_node(std::shared_ptr<ModelNode> parent, const tinygltf::Node &node, const uint32_t node_index, std::shared_ptr<Model> model,
                   const float global_scale);

    void load_skins(std::shared_ptr<Model> model);

    void load_textures(std::shared_ptr<Model> model);

    VkSamplerAddressMode get_wrap_mode(const int32_t wrap_mode);

    VkFilter get_filter_mode(const int32_t filter_mode);

    void load_texture_samplers(std::shared_ptr<Model> model);

    void load_materials(std::shared_ptr<Model> model);

    void load_animations(std::shared_ptr<Model> model);

    void render_node(std::shared_ptr<ModelNode> node, VkCommandBuffer command_buffer, VkPipelineLayout pipeline_layout, std::size_t current_image_index);

    void calculate_bounding_box(std::shared_ptr<Model> model, std::shared_ptr<ModelNode> node, std::shared_ptr<ModelNode> parent);

    void get_scene_dimensions(std::shared_ptr<Model> model);

    void update_animation(std::shared_ptr<Model> model, const uint32_t index, const float time);

    std::shared_ptr<ModelNode> find_node(std::shared_ptr<ModelNode> parent, const uint32_t index);

    std::shared_ptr<ModelNode> node_from_index(std::shared_ptr<Model> model, const uint32_t index);
};

} // namespace inexor::vulkan_renderer::gltf_model
