#include "inexor/vulkan-renderer/gltf/model.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::gltf {

Model::Model(const wrapper::Device &device, const ModelFile &model_file, float scale, glm::mat4 projection,
             glm::mat4 model)
    : m_device(device), m_model(model_file.model()), m_name(model_file.model_name()),
      m_default_scene_index(std::nullopt), m_model_scale(scale) {
    assert(m_device.device());

    // TODO: Refactor
    m_shader_data.model = model;
    m_shader_data.projection = projection;

    load_textures();
    load_materials();
    load_nodes();
    load_animations();
}

void Model::load_textures() {

    spdlog::trace("Loading {} glTF2 model texture indices", m_model.textures.size());

    // Preallocate memory for the texture indices.
    m_texture_indices.reserve(m_model.textures.size());

    for (const auto &texture : m_model.textures) {
        m_texture_indices.emplace_back(texture.source);
    }

    spdlog::trace("Loading {} texture samplers.", m_model.samplers.size());

    // Preallocate memory for the texture samplers.
    m_texture_samplers.reserve(m_model.samplers.size());

    for (const auto &sampler : m_model.samplers) {
        m_texture_samplers.emplace_back(sampler.minFilter, sampler.magFilter, sampler.wrapS, sampler.wrapT);
    }

    spdlog::trace("Loading {} textures from glTF2 model.", m_model.images.size());

    // Preallocate memory for the model images.
    m_textures.reserve(m_model.images.size());

    for (const auto &texture : m_model.textures) {
        auto texture_image = m_model.images[texture.source];

        TextureSampler new_sampler{};

        if (texture.sampler == -1) {
            // No sampler specified, use a default one.
            new_sampler = m_default_texture_sampler;
        } else {
            new_sampler = m_texture_samplers.at(texture.sampler);
        }

        // The size of the texture if it had 4 channels (rgba).
        const std::size_t texture_size =
            static_cast<std::size_t>(texture_image.width) * static_cast<std::size_t>(texture_image.height) * 4;

        // We need to convert RGB-only images to RGBA format, because most devices don't support rgb-formats in Vulkan.
        switch (texture_image.component) {
        case 3: {
            std::vector<std::array<std::uint32_t, 3>> rgb_source;
            rgb_source.reserve(texture_size);

            // Copy the memory into the vector, so we can safely perform std::transform on it.
            std::memcpy(rgb_source.data(), texture_image.image.data(), texture_size);

            std::vector<std::array<std::uint32_t, 4>> rgba_target;
            rgba_target.reserve(texture_size);

            std::transform(rgb_source.begin(), rgb_source.end(), rgba_target.begin(),
                           [](const std::array<std::uint32_t, 3> a) {
                               // convert RGB-only to RGBA.
                               return std::array<std::uint32_t, 4>{a[0], a[1], a[2], 1};
                           });

            std::string texture_name = texture.name.empty() ? "glTF2 model texture" : texture.name;

            // Create a texture using the data which was converted to RGBA.
            m_textures.emplace_back(m_device, new_sampler, rgba_target.data(), texture_size, texture_image.width,
                                    texture_image.height, texture_image.component, 1, texture_name);
            break;
        }
        case 4: {
            std::string texture_name = texture.name.empty() ? "glTF2 model texture" : texture.name;

            // Create a texture using RGBA data.
            m_textures.emplace_back(m_device, new_sampler, texture_image.image.data(), texture_size,
                                    texture_image.width, texture_image.height, texture_image.component, 1,
                                    texture_name);
            break;
        }
        default: {
            spdlog::error("Can't load texture with {} channels!", texture_image.component);
            spdlog::warn("Generating error texture as a replacement.");

            // Generate an error texture.
            // m_textures.emplace_back(m_device, m_default_texture_sampler, wrapper::CpuTexture());
            break;
        }
        }

        // TODO: Generate mipmaps!
    }
}

void Model::load_materials() {
    spdlog::trace("Loading {} glTF2 model materials", m_model.materials.size());

    // Preallocate memory for the model materials and one default material.
    m_materials.resize(1 + m_model.materials.size());

    std::unordered_map<std::string, bool> unsupported_features{};

    for (const auto &material : m_model.materials) {
        for (const auto &material_value : material.values) {

            const auto &material_name = material_value.first;

            ModelMaterial model_material{};

            if (material_name == "baseColorFactor") {
                model_material.base_color_factor = glm::make_vec4(material_value.second.ColorFactor().data());
            } else if (material_name == "baseColorTexture") {
                model_material.base_color_texture_index = material_value.second.TextureIndex();
            } else {
                // If the material name is not supported, add it to the list of unsupported materials.
                // This list will be printed at the end of material loading.
                unsupported_features[material_name] = true;
            }

            m_materials.emplace_back(model_material);
        }
    }

    // Print the list of unsupported materials.
    for (auto const &[key, val] : unsupported_features) {
        spdlog::warn("Material {} not supported!", key);
    }

    // Add a default material at the end for models with no materials assigned to them.
    m_materials.emplace_back();

    // TODO: Support glTF extensions.
}

ModelNode *Model::find_node(ModelNode *parent, const std::uint32_t index) {
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

ModelNode *Model::node_from_index(const std::uint32_t index) {
    ModelNode *node_found = nullptr;
    for (auto &node : m_nodes) {
        node_found = find_node(&node, index);
        if (node_found != nullptr) {
            break;
        }
    }
    return node_found;
}

void Model::load_node(ModelNode *parent, const tinygltf::Node &node, const std::uint32_t scene_index,
                      const std::uint32_t node_index) {

    auto &vertex_buffer = m_scenes[scene_index].vertices;
    auto &index_buffer = m_scenes[scene_index].indices;

    ModelNode new_node;
    new_node.name = node.name;
    new_node.parent = parent;
    new_node.index = node_index;
    new_node.skin_index = node.skin;
    new_node.matrix = glm::mat4(1.0f);

    // The local node matrix is either made up from translation, rotation, scale or a 4x4 matrix.
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
            // Load child nodes recursively.
            load_node(&new_node, node, scene_index, node.children[child_index]);
        }
    }

    if (node.name == "Light") {
        spdlog::trace("Loading lights from glTF2 models is not supported yet.");
    } else if (node.name == "Camera") {
        spdlog::trace("Loading cameras from glTF2 models is not supported yet.");
    } else if (node.mesh > -1) {

        // These definitions help us to shorten the following code passages.
        const auto &mesh = m_model.meshes[node.mesh];
        const auto &accessors = m_model.accessors;
        const auto &buffers = m_model.buffers;
        const auto &buffer_views = m_model.bufferViews;

        for (const auto &primitive : mesh.primitives) {
            const auto attr = primitive.attributes;

            auto vertex_start = static_cast<uint32_t>(vertex_buffer.size());
            auto index_start = static_cast<uint32_t>(index_buffer.size());

            std::uint32_t vertex_count = 0;
            std::uint32_t index_count = 0;

            glm::vec3 pos_min{};
            glm::vec3 pos_max{};

            bool has_skin = false;
            bool has_indices = primitive.indices > -1;

            // Vertices
            {
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

                // Position attribute is required!
                if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
                    throw InexorException("Error: glTF2 model " + m_name + " is missing position attribute!");
                }

                const auto &pos_accessor = m_model.accessors[primitive.attributes.find("POSITION")->second];
                const auto &pos_view = m_model.bufferViews[pos_accessor.bufferView];

                buffer_pos = reinterpret_cast<const float *>( // NOLINT
                    &(m_model.buffers[pos_view.buffer].data[pos_accessor.byteOffset + pos_view.byteOffset]));

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
                    buffer_joints =
                        &(buffers[joint_view.buffer].data[joint_accessor.byteOffset + joint_view.byteOffset]);

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

                for (std::size_t v = 0; v < pos_accessor.count; v++) {
                    ModelVertex vert{};
                    vert.pos = glm::vec4(glm::make_vec3(&buffer_pos[v * position_byte_stride]), 1.0f); // NOLINT

                    if (buffer_normals != nullptr) {
                        vert.normal = glm::normalize(
                            glm::vec3(glm::make_vec3(&buffer_normals[v * normal_byte_stride]))); // NOLINT
                    } else {
                        vert.normal = glm::normalize(glm::vec3(glm::vec3(0.0f)));
                    }

                    if (buffer_texture_coord_set_0 != nullptr) {
                        vert.uv[0] = glm::make_vec2(&buffer_texture_coord_set_0[v * uv0_byte_stride]); // NOLINT
                    } else {
                        vert.uv[0] = glm::vec3(0.0f);
                    }

                    if (buffer_texture_coord_set_1 != nullptr) {
                        vert.uv[1] = glm::make_vec2(&buffer_texture_coord_set_1[v * uv1_byte_stride]); // NOLINT
                    } else {
                        vert.uv[1] = glm::vec3(0.0f);
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

                    // Fix for all zero weights.
                    if (glm::length(vert.weight) == 0.0f) {
                        vert.weight = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
                    }

                    vertex_buffer.push_back(vert);
                }
            }

            // Indices
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
                        index_buffer.push_back(buf[index] + vertex_start); // NOLINT
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const auto *buf = static_cast<const std::uint16_t *>(index_data_pointer);
                    for (std::size_t index = 0; index < accessor.count; index++) {
                        index_buffer.push_back(buf[index] + vertex_start); // NOLINT
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    const auto *buf = static_cast<const std::uint8_t *>(index_data_pointer);
                    for (std::size_t index = 0; index < accessor.count; index++) {
                        index_buffer.push_back(buf[index] + vertex_start); // NOLINT
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

            new_node.mesh.push_back(new_primitive);
        }

        // TODO: Is something missing?

    } else {
        spdlog::warn("Unknown node type {} is not supported!", node.name);
    }

    if (parent != nullptr) {
        parent->children.push_back(new_node);
    } else {
        m_nodes.push_back(new_node);
    }
}

void Model::load_nodes() {
    if (m_model.scenes.empty()) {
        spdlog::trace("The glTF2 model does not contain nodes.");
        return;
    }

    spdlog::trace("Loading {} glTF2 model scenes", m_model.scenes.size());

    // Preallocate memory for the model.
    // Call resize and not reserve to make sure constructor is called.
    m_scenes.resize(m_model.scenes.size());

    if (m_model.defaultScene > -1) {
        m_default_scene_index = m_model.defaultScene;
        spdlog::trace("Default scene index: {}", m_model.defaultScene);
    } else {
        spdlog::trace("No default scene index specified.");
    }

    for (std::size_t scene_index = 0; scene_index < m_scenes.size(); scene_index++) {
        const auto &scene = m_model.scenes[scene_index];

        for (std::size_t node_index = 0; node_index < scene.nodes.size(); node_index++) {
            const auto &node = m_model.nodes[scene.nodes[node_index]];
            load_node(nullptr, node, scene.nodes[node_index], static_cast<std::uint32_t>(scene_index));
        }
    }
}

void Model::load_animations() {
    if (m_model.animations.empty()) {
        spdlog::trace("The glTF2 model does not contain animations");
        return;
    }

    spdlog::trace("Loading {} glTF2 model animations", m_model.animations.size());

    for (const auto &animation : m_model.animations) {

        ModelAnimation new_animation{};
        new_animation.name = animation.name;

        if (new_animation.name.empty()) {
            new_animation.name = std::to_string(m_model.animations.size());
        }

        // Samplers
        for (const auto &samp : animation.samplers) {
            ModelAnimationSampler sampler{};

            if (samp.interpolation == "LINEAR") {
                sampler.interpolation = ModelAnimationSampler::InterpolationType::LINEAR;
            }
            if (samp.interpolation == "STEP") {
                sampler.interpolation = ModelAnimationSampler::InterpolationType::STEP;
            }
            if (samp.interpolation == "CUBICSPLINE") {
                sampler.interpolation = ModelAnimationSampler::InterpolationType::CUBICSPLINE;
            }

            // Read sampler input time values
            {
                const auto &accessor = m_model.accessors[samp.input];
                const auto &buffer_view = m_model.bufferViews[accessor.bufferView];
                const auto &buffer = m_model.buffers[buffer_view.buffer];

                assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                const void *data_pointer = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];
                const auto *buf = static_cast<const float *>(data_pointer);
                for (std::size_t index = 0; index < accessor.count; index++) {
                    sampler.inputs.push_back(buf[index]); // NOLINT
                }

                for (auto input : sampler.inputs) {
                    if (input < new_animation.start) {
                        new_animation.start = input;
                    }
                    if (input > new_animation.end) {
                        new_animation.end = input;
                    }
                }
            }

            // Read sampler output T/R/S values
            {
                const auto &accessor = m_model.accessors[samp.output];
                const auto &buffer_view = m_model.bufferViews[accessor.bufferView];
                const auto &buffer = m_model.buffers[buffer_view.buffer];

                assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

                const void *data_pointer = &buffer.data[accessor.byteOffset + buffer_view.byteOffset];

                switch (accessor.type) {
                case TINYGLTF_TYPE_VEC3: {
                    const auto *buf = static_cast<const glm::vec3 *>(data_pointer);
                    for (std::size_t index = 0; index < accessor.count; index++) {
                        sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f)); // NOLINT
                    }
                    break;
                }
                case TINYGLTF_TYPE_VEC4: {
                    const auto *buf = static_cast<const glm::vec4 *>(data_pointer);
                    for (std::size_t index = 0; index < accessor.count; index++) {
                        sampler.outputsVec4.push_back(buf[index]); // NOLINT
                    }
                    break;
                }
                default: {
                    spdlog::error("Unknown animation accessor type");
                    break;
                }
                }
            }

            new_animation.samplers.push_back(sampler);
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

            new_channel.samplerIndex = source.sampler;
            new_channel.node = node_from_index(source.target_node);

            if (new_channel.node == nullptr) {
                continue;
            }

            new_animation.channels.push_back(new_channel);
        }

        animations.push_back(new_animation);
    }
}

} // namespace inexor::vulkan_renderer::gltf
