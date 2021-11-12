#include "inexor/vulkan-renderer/gltf/gltf_gpu_data.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::gltf {

ModelGpuData::ModelGpuData(RenderGraph *render_graph, const ModelCpuData &model_cpu_data, const glm::mat4 &model_matrix,
                           const glm::mat4 &proj_matrix) {

    const auto &model = model_cpu_data.model();
    load_textures(render_graph->device_wrapper(), model);
    load_materials(model);
    load_nodes(render_graph->device_wrapper(), model);
    load_animations(model);
    load_skins(model);

    setup_rendering_resources(render_graph);
}

ModelGpuData::ModelGpuData(ModelGpuData &&other) noexcept {
    m_unsupported_node_types = std::move(other.m_unsupported_node_types);
    m_texture_indices = std::move(other.m_texture_indices);
    m_materials = std::move(other.m_materials);
    m_nodes = std::move(other.m_nodes);
    m_linear_nodes = std::move(other.m_linear_nodes);
    animations = std::move(other.animations);
    m_skins = std::move(other.m_skins);
    m_textures = std::move(other.m_textures);
    m_texture_samplers = std::move(other.m_texture_samplers);
    m_scene = std::move(other.m_scene);
    m_shader_values = std::move(other.m_shader_values);
    m_uniform_buffer = std::exchange(other.m_uniform_buffer, nullptr);
    m_default_texture_sampler = std::move(other.m_default_texture_sampler);
}

void ModelGpuData::load_textures(const wrapper::Device &device, const tinygltf::Model &model) {
    spdlog::trace("Loading {} glTF2 model texture indices", model.textures.size());

    m_texture_indices.reserve(model.textures.size());

    for (const auto &texture : model.textures) {
        m_texture_indices.emplace_back(texture.source);
    }

    spdlog::trace("Loading {} texture samplers.", model.samplers.size());

    m_texture_samplers.reserve(model.samplers.size());

    for (const auto &sampler : model.samplers) {
        m_texture_samplers.emplace_back(sampler.minFilter, sampler.magFilter, sampler.wrapS, sampler.wrapT);
    }

    spdlog::trace("Loading {} textures from glTF2 model.", model.images.size());

    m_textures.reserve(model.images.size());

    for (const auto &texture : model.textures) {
        auto &texture_image = model.images[texture.source];

        const auto &new_sampler =
            (texture.sampler == -1) ? m_default_texture_sampler : m_texture_samplers.at(texture.sampler);

        // The size of the texture if it had 4 channels (RGBA)
        const std::size_t texture_size =
            static_cast<std::size_t>(texture_image.width) * static_cast<std::size_t>(texture_image.height) * 4;

        // We need to convert RGB-only images to RGBA format (most devices don't support RGB-formats in Vulkan)
        switch (texture_image.component) {
        case 3: {
            std::vector<std::array<std::uint32_t, 3>> rgb_source;
            rgb_source.reserve(texture_size);

            // Copy the memory into the vector, so we can safely perform std::transform on it.
            std::memcpy(rgb_source.data(), texture_image.image.data(), texture_size);

            std::vector<std::array<std::uint32_t, 4>> rgba_target;
            rgba_target.reserve(texture_size);

            // Convert RGB-only to RGBA
            std::transform(rgb_source.begin(), rgb_source.end(), rgba_target.begin(),
                           [](const std::array<std::uint32_t, 3> rgb_values) {
                               return std::array<std::uint32_t, 4>{rgb_values[0], rgb_values[1], rgb_values[2], 1};
                           });

            std::string texture_name = texture.name.empty() ? "glTF2 model texture" : texture.name;

            // Create a texture using the data which was converted to RGBA
            m_textures.emplace_back(device, new_sampler, rgba_target.data(), texture_size, texture_image.width,
                                    texture_image.height, texture_image.component, 1, texture_name);
            break;
        }
        case 4: {
            std::string texture_name = texture.name.empty() ? "glTF2 model texture" : texture.name;

            // Create a texture using RGBA data
            m_textures.emplace_back(device, new_sampler, texture_image.image.data(), texture_size, texture_image.width,
                                    texture_image.height, texture_image.component, 1, texture_name);
            break;
        }
        default: {
            spdlog::error("Can't load texture with {} channels!", texture_image.component);
            spdlog::error("Generating error texture as a replacement.");

            // Generate an error texture (chessboard pattern)
            m_textures.emplace_back(device, m_default_texture_sampler, wrapper::CpuTexture());
            break;
        }
        }

        // TODO: Generate mipmaps!
    }
}

void ModelGpuData::load_materials(const tinygltf::Model &model) {
    spdlog::trace("Loading {} glTF2 model materials", model.materials.size());

    m_materials.resize(1 + model.materials.size());

    std::unordered_map<std::string, bool> unsupported_material_features{};

    for (const auto &material : model.materials) {
        ModelMaterial new_material{};

        // Load physically based rendering (pbr) material values
        for (const auto &[name, value] : material.values) {
            if (name == "baseColorTexture") {
                new_material.base_color_texture = &m_textures[value.TextureIndex()];
                new_material.texture_coordinate_set.base_color = value.TextureTexCoord();
            } else if (name == "metallicRoughnessTexture") {
                new_material.metallic_roughness_texture = &m_textures[value.TextureIndex()];
                new_material.texture_coordinate_set.metallic_roughness = value.TextureTexCoord();
            } else if (name == "roughnessFactor") {
                new_material.roughness_factor = static_cast<float>(value.Factor());
            } else if (name == "metallicFactor") {
                new_material.metallic_factor = static_cast<float>(value.Factor());
            } else if (name == "baseColorFactor") {
                new_material.base_color_factor = glm::make_vec4(value.ColorFactor().data());
            } else {
                // Remember this unsupported feature
                unsupported_material_features[name] = true;
            }
        }

        // Load additional material values
        for (const auto &[name, value] : material.additionalValues) {
            if (name == "normalTexture") {
                new_material.normal_texture = &m_textures[value.TextureIndex()];
                new_material.texture_coordinate_set.normal = value.TextureTexCoord();
            } else if (name == "emissiveTexture") {
                new_material.emissive_texture = &m_textures[value.TextureIndex()];
                new_material.texture_coordinate_set.emissive = value.TextureTexCoord();
            } else if (name == "occlusionTexture") {
                new_material.occlusion_texture = &m_textures[value.TextureIndex()];
                new_material.texture_coordinate_set.occlusion = value.TextureTexCoord();
            } else if (name == "alphaMode") {
                if (value.string_value == "BLEND") {
                    new_material.alpha_mode = ModelMaterial::AlphaMode::ALPHAMODE_BLEND;
                }
                if (value.string_value == "MASK") {
                    new_material.alpha_cutoff = 0.5f;
                    new_material.alpha_mode = ModelMaterial::AlphaMode::ALPHAMODE_MASK;
                }
            } else if (name == "alphaCutoff") {
                new_material.alpha_cutoff = static_cast<float>(value.Factor());
            } else if (name == "emissiveFactor") {
                new_material.emissive_factor = glm::vec4(glm::make_vec3(value.ColorFactor().data()), 1.0);
                new_material.emissive_factor = glm::vec4(0.0f);
            } else {
                // Remember this unsupported feature
                unsupported_material_features[name] = true;
            }

            // Load materials from extensions
            if (material.extensions.find("KHR_materials_pbrSpecularGlossiness") != material.extensions.end()) {
                const auto &extension = material.extensions.find("KHR_materials_pbrSpecularGlossiness");

                if (extension->second.Has("specularGlossinessTexture")) {
                    const auto &index = extension->second.Get("specularGlossinessTexture").Get("index");
                    new_material.extension.specular_glossiness_texture = &m_textures[index.Get<int>()];
                    const auto &texture_coordinate_set =
                        extension->second.Get("specularGlossinessTexture").Get("texCoord");
                    new_material.texture_coordinate_set.specular_glossiness = texture_coordinate_set.Get<int>();
                    new_material.pbr_workflows.specular_glossiness = true;
                }

                if (extension->second.Has("diffuseTexture")) {
                    const auto &index = extension->second.Get("diffuseTexture").Get("index");
                    new_material.extension.diffuse_texture = &m_textures[index.Get<int>()];
                }

                if (extension->second.Has("diffuseFactor")) {
                    const auto &factor = extension->second.Get("diffuseFactor");
                    for (std::uint32_t i = 0; i < factor.ArrayLen(); i++) {
                        const auto &val = factor.Get(i);
                        new_material.extension.diffuse_factor[i] =
                            val.IsNumber() ? static_cast<float>(val.Get<double>()) : static_cast<float>(val.Get<int>());
                    }
                }

                if (extension->second.Has("specularFactor")) {
                    const auto &factor = extension->second.Get("specularFactor");
                    for (std::uint32_t i = 0; i < factor.ArrayLen(); i++) {
                        const auto val = factor.Get(i);
                        new_material.extension.specular_factor[i] =
                            val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
                    }
                }
            }
        }

        m_materials.push_back(new_material);
    }

    for (const auto &[name, value] : unsupported_material_features) {
        spdlog::warn("Material feature {} not supported!", name);
    }

    // TODO: Check during loading of the mesh if we really need this!
    // Add a default material at the end of the list for meshes with no material assigned
    m_materials.emplace_back();
}

ModelNode *ModelGpuData::find_node(ModelNode *parent, const std::uint32_t index) {
    ModelNode *node_found = nullptr;
    if (parent->index == index) {
        return parent;
    }
    for (auto &child : parent->children) {
        node_found = find_node(&child, index);
        if (node_found != nullptr) {
            break;
        }
    }
    return node_found;
}

ModelNode *ModelGpuData::node_from_index(const std::uint32_t index) {
    ModelNode *node_found = nullptr;
    for (auto &node : m_nodes) {
        node_found = find_node(&node, index);
        if (node_found != nullptr) {
            break;
        }
    }
    return node_found;
}

void ModelGpuData::load_node(const wrapper::Device &device_wrapper, const tinygltf::Model &model, ModelNode *parent,
                             const tinygltf::Node &node, const std::uint32_t scene_index,
                             const std::uint32_t node_index) {
    ModelNode new_node;
    new_node.name = node.name;
    new_node.parent = parent;
    new_node.index = node_index;
    new_node.skin_index = node.skin;
    new_node.matrix = glm::mat4(1.0f);

    // The local node matrix is either made up from translation, rotation, scale or a 4x4 matrix
    if (node.translation.size() == 3) {
        new_node.translation = glm::make_vec3(node.translation.data());
    }

    if (node.rotation.size() == 4) {
        glm::quat rotation_quaternion = glm::make_quat(node.rotation.data());
        new_node.rotation = glm::mat4(rotation_quaternion);
    }

    if (node.scale.size() == 3) {
        new_node.scale = glm::make_vec3(node.scale.data());
    }

    if (node.matrix.size() == 16) {
        new_node.matrix = glm::make_mat4x4(node.matrix.data());
    }

    if (!node.children.empty()) {
        for (std::size_t child_index = 0; child_index < node.children.size(); child_index++) {
            load_node(device_wrapper, model, &new_node, model.nodes.at(node.children.at(child_index)), scene_index,
                      node.children.at(child_index));
        }
    }

    // Those will be supported in the future, so we don't add them to the unordered set of unsupported features
    if (node.name == "Light") {
        spdlog::trace("Loading lights from glTF2 models is not supported yet.");
    } else if (node.name == "Camera") {
        spdlog::trace("Loading cameras from glTF2 models is not supported yet.");
    }

    if (node.mesh > -1) {
        const auto &mesh = model.meshes[node.mesh];
        const auto &accessors = model.accessors;
        const auto &buffers = model.buffers;
        const auto &buffer_views = model.bufferViews;

        std::unique_ptr<ModelMesh> new_mesh = std::make_unique<ModelMesh>(device_wrapper, new_node.matrix);

        for (const auto &primitive : mesh.primitives) {
            const auto attr = primitive.attributes;

            auto vertex_start = static_cast<uint32_t>(m_vertices.size());
            auto index_start = static_cast<uint32_t>(m_indices.size());

            std::uint32_t vertex_count = 0;
            std::uint32_t index_count = 0;

            glm::vec3 pos_min{};
            glm::vec3 pos_max{};

            bool has_skin = false;
            bool has_indices = primitive.indices > -1;

            const float *buffer_pos = nullptr;
            const float *buffer_normals = nullptr;
            const float *buffer_texture_coord_set_0 = nullptr;
            const float *buffer_texture_coord_set_1 = nullptr;
            const void *buffer_joints = nullptr;
            const float *buffer_weights = nullptr;

            std::uint32_t position_byte_stride = 0;
            std::uint32_t normal_byte_stride = 0;
            std::uint32_t uv0_byte_stride = 0;
            std::uint32_t uv1_byte_stride = 0;
            std::uint32_t joint_byte_stride = 0;
            std::uint32_t weight_byte_stride = 0;
            std::uint32_t joint_component_type = 0;

            // TODO: Position attribute is required!

            const auto &pos_accessor = model.accessors[primitive.attributes.find("POSITION")->second];
            const auto &pos_view = model.bufferViews[pos_accessor.bufferView];

            buffer_pos = reinterpret_cast<const float *>( // NOLINT
                &(model.buffers[pos_view.buffer].data[pos_accessor.byteOffset + pos_view.byteOffset]));

            pos_min = glm::vec3(pos_accessor.minValues[0], pos_accessor.minValues[1], pos_accessor.minValues[2]);
            pos_max = glm::vec3(pos_accessor.maxValues[0], pos_accessor.maxValues[1], pos_accessor.maxValues[2]);

            vertex_count = static_cast<uint32_t>(pos_accessor.count);

            if (pos_accessor.ByteStride(pos_view)) {
                position_byte_stride = (pos_accessor.ByteStride(pos_view) / sizeof(float));
            } else {
                position_byte_stride = 3;
            }

            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                const auto &norm_accessor = accessors[primitive.attributes.find("NORMAL")->second];
                const auto &norm_view = buffer_views[norm_accessor.bufferView];

                buffer_normals = reinterpret_cast<const float *>( // NOLINT
                    &(buffers[norm_view.buffer].data[norm_accessor.byteOffset + norm_view.byteOffset]));

                if (norm_accessor.ByteStride(norm_view)) {
                    normal_byte_stride = (norm_accessor.ByteStride(norm_view) / sizeof(float));
                } else {
                    normal_byte_stride = 3;
                }
            }

            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                const auto &uv_accessor = accessors[primitive.attributes.find("TEXCOORD_0")->second];
                const auto &uv_view = buffer_views[uv_accessor.bufferView];

                buffer_texture_coord_set_0 = reinterpret_cast<const float *>( // NOLINT
                    &(buffers[uv_view.buffer].data[uv_accessor.byteOffset + uv_view.byteOffset]));

                if (uv_accessor.ByteStride(uv_view)) {
                    uv0_byte_stride = (uv_accessor.ByteStride(uv_view) / sizeof(float));
                } else {
                    uv0_byte_stride = 2;
                }
            }
            if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end()) {
                const auto &uv_accessor = accessors[primitive.attributes.find("TEXCOORD_1")->second];
                const auto &uv_view = buffer_views[uv_accessor.bufferView];

                buffer_texture_coord_set_1 = reinterpret_cast<const float *>( // NOLINT
                    &(buffers[uv_view.buffer].data[uv_accessor.byteOffset + uv_view.byteOffset]));

                if (uv_accessor.ByteStride(uv_view)) {
                    uv1_byte_stride = (uv_accessor.ByteStride(uv_view) / sizeof(float));
                } else {
                    uv1_byte_stride = 2;
                }
            }

            if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
                const auto &joint_accessor = accessors[primitive.attributes.find("JOINTS_0")->second];
                const auto &joint_view = buffer_views[joint_accessor.bufferView];
                buffer_joints = &(buffers[joint_view.buffer].data[joint_accessor.byteOffset + joint_view.byteOffset]);

                joint_component_type = joint_accessor.componentType;

                if (joint_accessor.ByteStride(joint_view)) {
                    joint_byte_stride = (joint_accessor.ByteStride(joint_view) /
                                         tinygltf::GetComponentSizeInBytes(joint_component_type));
                } else {
                    joint_byte_stride = 4;
                }
            }

            if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
                const auto &weight_accessor = accessors[primitive.attributes.find("WEIGHTS_0")->second];
                const auto &weight_view = buffer_views[weight_accessor.bufferView];

                buffer_weights = reinterpret_cast<const float *>( // NOLINT
                    &(buffers[weight_view.buffer].data[weight_accessor.byteOffset + weight_view.byteOffset]));

                if (weight_accessor.ByteStride(weight_view)) {
                    weight_byte_stride = (weight_accessor.ByteStride(weight_view) / sizeof(float));
                } else {
                    weight_byte_stride = 4;
                }
            }

            has_skin = (buffer_joints != nullptr && buffer_weights != nullptr);

            m_vertices.reserve(pos_accessor.count);

            for (std::size_t v = 0; v < pos_accessor.count; v++) {
                ModelVertex vert{};
                vert.pos = glm::vec4(glm::make_vec3(&buffer_pos[v * position_byte_stride]), 1.0f); // NOLINT

                if (buffer_normals != nullptr) {
                    vert.normal =
                        glm::normalize(glm::vec3(glm::make_vec3(&buffer_normals[v * normal_byte_stride]))); // NOLINT
                } else {
                    vert.normal = glm::normalize(glm::vec3(glm::vec3(0.0f)));
                }

                if (buffer_texture_coord_set_0 != nullptr) {
                    vert.uv0 = glm::make_vec2(&buffer_texture_coord_set_0[v * uv0_byte_stride]); // NOLINT
                } else {
                    vert.uv0 = glm::vec3(0.0f);
                }

                if (buffer_texture_coord_set_1 != nullptr) {
                    vert.uv1 = glm::make_vec2(&buffer_texture_coord_set_1[v * uv1_byte_stride]); // NOLINT
                } else {
                    vert.uv1 = glm::vec3(0.0f);
                }

                if (has_skin) {
                    switch (joint_component_type) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                        const std::uint16_t *buf = static_cast<const std::uint16_t *>(buffer_joints); // NOLINT
                        vert.joint = glm::vec4(glm::make_vec4(&buf[v * joint_byte_stride]));          // NOLINT
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                        const std::uint8_t *buf = static_cast<const std::uint8_t *>(buffer_joints); // NOLINT
                        vert.joint = glm::vec4(glm::make_vec4(&buf[v * joint_byte_stride]));        // NOLINT
                        break;
                    }
                    default:
                        spdlog::warn("Joint component type {} is not supported!", joint_component_type);
                        break;
                    }
                } else {
                    vert.joint = glm::vec4(0.0f);
                }

                if (has_skin) {
                    vert.weight = glm::make_vec4(&buffer_weights[v * weight_byte_stride]); // NOLINT
                } else {
                    vert.weight = glm::vec4(0.0f);
                }

                if (glm::length(vert.weight) == 0.0f) {
                    // Fix for all zero weights
                    vert.weight = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
                }

                m_vertices.push_back(vert);
            }

            if (has_indices) {
                const auto &accessor = accessors[primitive.indices > -1 ? primitive.indices : 0];
                const auto &buffer_view = buffer_views[accessor.bufferView];
                const auto &buffer = buffers[buffer_view.buffer];

                index_count = static_cast<std::uint32_t>(accessor.count);

                const void *index_data_pointer = &(buffer.data[accessor.byteOffset + buffer_view.byteOffset]);

                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    const auto *buf = static_cast<const std::uint32_t *>(index_data_pointer);
                    for (std::size_t index = 0; index < accessor.count; index++) {
                        m_indices.push_back(buf[index] + vertex_start); // NOLINT
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const auto *buf = static_cast<const std::uint16_t *>(index_data_pointer);
                    for (std::size_t index = 0; index < accessor.count; index++) {
                        m_indices.push_back(buf[index] + vertex_start); // NOLINT
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    const auto *buf = static_cast<const std::uint8_t *>(index_data_pointer);
                    for (std::size_t index = 0; index < accessor.count; index++) {
                        m_indices.push_back(buf[index] + vertex_start); // NOLINT
                    }
                    break;
                }
                default:
                    spdlog::error("Index component type {} is not supported!", accessor.componentType);
                    return;
                }
            }

            ModelPrimitive new_primitive(index_start, index_count, vertex_count,
                                         primitive.material > -1 ? m_materials[primitive.material]
                                                                 : m_materials.back());

            new_primitive.set_bbox(pos_min, pos_max);

            new_mesh->primitives.push_back(new_primitive);
        }

        // TODO: Is this technically correct?
        // Mesh BB from BBs of primitives
        for (auto p : new_mesh->primitives) {
            if (p.bbox().valid && !new_mesh->bb.valid) {
                new_mesh->bb = p.bbox();
                new_mesh->bb.valid = true;
            }
            new_mesh->bb.min = glm::min(new_mesh->bb.min, p.bbox().min);
            new_mesh->bb.max = glm::max(new_mesh->bb.max, p.bbox().max);
        }

        new_node.mesh = std::move(new_mesh);
    } else {
        // Remember this unsupported glTF2 model node type
        m_unsupported_node_types[node.name] = true;
    }

    if (parent) {
        // TODO: Should we consider using emplace_back here?
        parent->children.push_back(std::move(new_node));
    } else {
        // TODO: Should we consider using emplace_back here?
        m_nodes.push_back(std::move(new_node));
    }

    // TODO: Should we consider using emplace_back here?
    m_linear_nodes.push_back(std::move(new_node));
}

void ModelGpuData::load_skins(const tinygltf::Model &model) {
    m_skins.reserve(model.skins.size());

    for (const auto &source : model.skins) {
        ModelSkin new_skin;

        new_skin.name = source.name;

        // Find skeleton root node
        if (source.skeleton > -1) {
            new_skin.skeleton_root = node_from_index(source.skeleton);
        }

        // Find joint nodes
        for (const auto &joint_index : source.joints) {
            auto *node = node_from_index(joint_index);
            if (node != nullptr) {
                new_skin.joints.push_back(node_from_index(joint_index));
            }
        }

        // Get inverse bind matrices from buffer
        if (source.inverseBindMatrices > -1) {
            const auto &accessor = model.accessors[source.inverseBindMatrices];
            const auto &bufferView = model.bufferViews[accessor.bufferView];
            const auto &buffer = model.buffers[bufferView.buffer];

            new_skin.inverse_bind_matrices.resize(accessor.count);

            const auto &dest_memory = new_skin.inverse_bind_matrices.data();
            const auto &source_memory = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
            const auto memory_size = accessor.count * sizeof(glm::mat4);

            std::memcpy(dest_memory, source_memory, memory_size);
        }

        m_skins.push_back(std::move(new_skin));
    }

    for (auto &node : m_linear_nodes) {
        // Assign skins
        if (node.skin_index > -1) {
            node.skin = &m_skins[node.skin_index];
        }
        // Initial pose
        if (node.mesh) {
            node.update();
        }
    }
}

void ModelGpuData::load_nodes(const wrapper::Device &device_wrapper, const tinygltf::Model &model) {
    if (model.scenes.empty()) {
        spdlog::trace("The glTF2 model does not contain nodes.");
        return;
    }

    spdlog::trace("Loading {} glTF2 model scenes", model.scenes.size());

    if (model.defaultScene > -1) {
        spdlog::trace("Default scene index: {}", model.defaultScene);
    } else {
        spdlog::trace("No default scene index specified.");
    }

    for (std::size_t scene_index = 0; scene_index < model.scenes.size(); scene_index++) {
        const auto &scene = model.scenes[scene_index];

        for (std::size_t node_index = 0; node_index < scene.nodes.size(); node_index++) {
            const auto &node = model.nodes[scene.nodes[node_index]];
            load_node(device_wrapper, model, nullptr, node, scene.nodes[node_index],
                      static_cast<std::uint32_t>(scene_index));
        }
    }
}

void ModelGpuData::load_animations(const tinygltf::Model &model) {
    if (model.animations.empty()) {
        spdlog::trace("The glTF2 model does not contain animations");
        return;
    }

    spdlog::trace("Loading {} glTF2 model animations", model.animations.size());

    for (const auto &animation : model.animations) {
        ModelAnimation new_animation{};
        new_animation.name = animation.name;

        if (new_animation.name.empty()) {
            new_animation.name = std::to_string(model.animations.size());
        }

        for (const auto &sampler : animation.samplers) {
            ModelAnimationSampler new_sampler{};

            if (sampler.interpolation == "LINEAR") {
                new_sampler.interpolation = ModelAnimationSampler::InterpolationType::LINEAR;
            }
            if (sampler.interpolation == "STEP") {
                new_sampler.interpolation = ModelAnimationSampler::InterpolationType::STEP;
            }
            if (sampler.interpolation == "CUBICSPLINE") {
                new_sampler.interpolation = ModelAnimationSampler::InterpolationType::CUBICSPLINE;
            }

            // Read sampler input time values
            {
                const auto &accessor = model.accessors[sampler.input];
                const auto &buffer_view = model.bufferViews[accessor.bufferView];
                const auto &buffer = model.buffers[buffer_view.buffer];

                // TODO: Change to an exception!
                assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                const void *data_pointer = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];
                const auto *buf = static_cast<const float *>(data_pointer);

                for (std::size_t index = 0; index < accessor.count; index++) {
                    new_sampler.inputs.push_back(buf[index]); // NOLINT
                }

                for (const auto &input : new_sampler.inputs) {
                    if (input < new_animation.start) {
                        new_animation.start = input;
                    }
                    if (input > new_animation.end) {
                        new_animation.end = input;
                    }
                }
            }

            // Read sampler output translation, rotation, scale (T/R/S) values
            {
                const auto &accessor = model.accessors[sampler.output];
                const auto &buffer_view = model.bufferViews[accessor.bufferView];
                const auto &buffer = model.buffers[buffer_view.buffer];

                // TODO: Change to an exception!
                assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                const void *data_pointer = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];

                switch (accessor.type) {
                case TINYGLTF_TYPE_VEC3: {
                    const auto *buf = static_cast<const glm::vec3 *>(data_pointer);
                    for (std::size_t index = 0; index < accessor.count; index++) {
                        new_sampler.outputs.push_back(glm::vec4(buf[index], 0.0f)); // NOLINT
                    }
                    break;
                }
                case TINYGLTF_TYPE_VEC4: {
                    const auto *buf = static_cast<const glm::vec4 *>(data_pointer);
                    for (std::size_t index = 0; index < accessor.count; index++) {
                        new_sampler.outputs.push_back(buf[index]); // NOLINT
                    }
                    break;
                }
                default: {
                    spdlog::error("Unknown animation accessor type");
                    break;
                }
                }
            }

            new_animation.samplers.push_back(new_sampler);
        }

        // Channels
        for (const auto &source : animation.channels) {
            ModelAnimationChannel new_channel{};

            if (source.target_path == "rotation") {
                new_channel.path = ModelAnimationChannel::PathType::ROTATION;
            }
            if (source.target_path == "translation") {
                new_channel.path = ModelAnimationChannel::PathType::TRANSLATION;
            }
            if (source.target_path == "scale") {
                new_channel.path = ModelAnimationChannel::PathType::SCALE;
            }
            if (source.target_path == "weights") {
                spdlog::warn("Weights in animations are not yet supported, skipping animation channel.");
                continue;
            }

            new_channel.sampler_index = source.sampler;
            new_channel.node = node_from_index(source.target_node);

            if (new_channel.node == nullptr) {
                continue;
            }

            new_animation.channels.push_back(std::move(new_channel));
        }

        animations.push_back(std::move(new_animation));
    }
}

void ModelGpuData::setup_rendering_resources(RenderGraph *render_graph) {
    // TODO: Implement!
}

} // namespace inexor::vulkan_renderer::gltf
