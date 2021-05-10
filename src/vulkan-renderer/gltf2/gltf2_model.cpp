#include "inexor/vulkan-renderer/gltf2/gltf2_model.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::gltf2 {

Model::Model(const wrapper::Device &device, tinygltf::Model &model) : m_device(device), m_model(model) {
    assert(m_device.device());
    load_textures();
    load_materials();
    load_nodes();
}

void Model::load_textures() {
    spdlog::info("Loading {} glTF2 model textures", m_model.images.size());

    // Preallocate memory for the model images.
    m_textures.reserve(m_model.images.size());

    for (auto texture : m_model.images) {
        // The size of the texture if it had 4 channels (rgba).
        const std::size_t texture_size = texture.width * texture.height * 4;

        // We need to convert RGB-only images to RGBA format, because most devices don't support rgb-formats in Vulkan.
        if (texture.component == 3) {
            std::vector<unsigned char> conversion_buffer;

            // Preallocate memory for a texture with 4 channels (RGBA).
            conversion_buffer.reserve(texture_size);

            // The pointer to which we will write data to.
            unsigned char *mem_target_rgba = conversion_buffer.data();

            // The pointer to the rgb data we read from.
            unsigned char *mem_source_rgb = &texture.image[0];

            for (size_t i = 0; i < texture.width * texture.height; ++i) {
                std::memcpy(mem_target_rgba, mem_source_rgb, sizeof(unsigned char) * 3);
                mem_target_rgba += 4;
                mem_source_rgb += 3;
            }

            m_textures.emplace_back(m_device, &conversion_buffer[0], texture_size, texture.width, texture.height,
                                    texture.component, 1, "glTF2 model texture");
        } else if (texture.component == 4) {
            m_textures.emplace_back(m_device, &texture.image[0], texture_size, texture.width, texture.height,
                                    texture.component, 1, "glTF2 model texture");
        } else {
            spdlog::error("Can't load texture from model file {}", m_file_name);

            // Generate an error texture (checkboard pattern of pink and black squares).
            m_textures.emplace_back(m_device, wrapper::CpuTexture());
        }
    }

    spdlog::info("Loading {} glTF2 model texture indices", m_model.textures.size());

    // Preallocate memory for the texture indices.
    m_texture_indices.reserve(m_model.textures.size());

    for (const auto &texture : m_model.textures) {
        m_texture_indices.emplace_back(texture.source);
    }
}

void Model::load_materials() {
    spdlog::info("Loading {} glTF2 model materials", m_model.materials.size());

    // Preallocate memory for the model materials.
    m_materials.resize(m_model.materials.size());

    ModelMaterial model_material{};

    for (auto material : m_model.materials) {
        if (material.values.find("baseColorFactor") != material.values.end()) {
            model_material.base_color_factor = glm::make_vec4(material.values["baseColorFactor"].ColorFactor().data());
        }
        if (material.values.find("baseColorTexture") != material.values.end()) {
            model_material.base_color_texture_index = material.values["baseColorTexture"].TextureIndex();
        }
        // TODO: Extract more material data from the glTF2 file as needed.

        m_materials.emplace_back(model_material);
    }
}

void Model::load_node(const tinygltf::Node &start_node, ModelNode *parent, std::vector<ModelVertex> &vertices,
                      std::vector<std::uint32_t> &indices) {
    ModelNode new_node{};
    new_node.matrix = glm::mat4(1.0f);

    // Get the local node matrix: It's either made up from translation, rotation, scale or a 4x4 matrix.
    if (start_node.translation.size() == 3) {
        new_node.matrix = glm::translate(new_node.matrix, glm::vec3(glm::make_vec3(start_node.translation.data())));
    }
    if (start_node.rotation.size() == 4) {
        glm::quat q = glm::make_quat(start_node.rotation.data());
        new_node.matrix *= glm::mat4(q);
    }
    if (start_node.scale.size() == 3) {
        new_node.matrix = glm::scale(new_node.matrix, glm::vec3(glm::make_vec3(start_node.scale.data())));
    }
    if (start_node.matrix.size() == 16) {
        new_node.matrix = glm::make_mat4x4(start_node.matrix.data());
    }

    // Load child nodes recursively.
    if (start_node.children.size() > 0) {
        for (const auto &child_node : start_node.children) {
            load_node(m_model.nodes[child_node], parent, vertices, indices);
        }
    }

    // If the node contains mesh data, we load vertices and indices from the buffers.
    // In glTF this is done via accessors and buffer views.
    if (start_node.mesh > -1) {
        const tinygltf::Mesh mesh = m_model.meshes[start_node.mesh];

        // Iterate through all primitives of this node's mesh.
        for (const auto primitive : mesh.primitives) {
            const auto attr = primitive.attributes;

            // Load Vertices.
            const float *position_buffer = nullptr;
            const float *normals_buffer = nullptr;
            const float *texture_coordinate_buffer = nullptr;

            size_t vertex_count = 0;

            // Get buffer data for vertex normals.
            if (attr.find("POSITION") != attr.end()) {
                const tinygltf::Accessor &accessor = m_model.accessors[attr.find("POSITION")->second];
                const tinygltf::BufferView &view = m_model.bufferViews[accessor.bufferView];
                position_buffer = reinterpret_cast<const float *>( // NOLINT
                    &(m_model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                vertex_count = accessor.count;
            }

            // Get buffer data for vertex normals.
            if (attr.find("NORMAL") != attr.end()) {
                const tinygltf::Accessor &accessor = m_model.accessors[attr.find("NORMAL")->second];
                const tinygltf::BufferView &view = m_model.bufferViews[accessor.bufferView];
                normals_buffer = reinterpret_cast<const float *>( // NOLINT
                    &(m_model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            }

            // Get buffer data for vertex texture coordinates.
            // glTF supports multiple sets, we only load the first one.
            if (attr.find("TEXCOORD_0") != attr.end()) {
                const tinygltf::Accessor &accessor = m_model.accessors[attr.find("TEXCOORD_0")->second];
                const tinygltf::BufferView &view = m_model.bufferViews[accessor.bufferView];
                texture_coordinate_buffer = reinterpret_cast<const float *>( // NOLINT
                    &(m_model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            }

            // Append data to model's vertex buffer
            for (size_t vertex_number = 0; vertex_number < vertex_count; vertex_number++) {
                ModelVertex new_vertex{};
                new_vertex.pos = glm::vec4(glm::make_vec3(&position_buffer[vertex_number * 3]), 1.0f);
                new_vertex.normal = glm::normalize(
                    glm::vec3(normals_buffer ? glm::make_vec3(&normals_buffer[vertex_number * 3]) : glm::vec3(0.0f)));
                new_vertex.uv = texture_coordinate_buffer
                                    ? glm::make_vec2(&texture_coordinate_buffer[vertex_number * 2])
                                    : glm::vec3(0.0f);
                new_vertex.color = glm::vec3(1.0f);

                vertices.push_back(new_vertex);
            }

            // Load indices.
            const auto &accessor = m_model.accessors[primitive.indices];
            const auto &buffer_view = m_model.bufferViews[accessor.bufferView];
            const auto &buffer = m_model.buffers[buffer_view.buffer];

            auto first_index = static_cast<std::uint32_t>(indices.size());
            auto vertex_start = static_cast<std::uint32_t>(vertices.size());
            auto index_count = static_cast<std::uint32_t>(accessor.count);

            // glTF2 supports different component types of indices.
            switch (accessor.componentType) {
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                std::vector<std::uint32_t> index_data;
                index_data.reserve(accessor.count);
                std::memcpy(index_data.data(), &buffer.data[accessor.byteOffset + buffer_view.byteOffset],
                            accessor.count * sizeof(std::uint32_t));

                for (std::size_t index = 0; index < accessor.count; index++) {
                    indices.push_back(index_data[index] + vertex_start);
                }
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                std::vector<std::uint16_t> index_data;
                index_data.reserve(accessor.count);
                std::memcpy(index_data.data(), &buffer.data[accessor.byteOffset + buffer_view.byteOffset],
                            accessor.count * sizeof(std::uint16_t));

                for (std::size_t index = 0; index < accessor.count; index++) {
                    indices.push_back(index_data[index] + vertex_start);
                }
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                std::vector<std::uint8_t> index_data;
                index_data.reserve(accessor.count);
                std::memcpy(index_data.data(), &buffer.data[accessor.byteOffset + buffer_view.byteOffset],
                            accessor.count * sizeof(std::uint8_t));

                for (std::size_t index = 0; index < accessor.count; index++) {
                    indices.push_back(index_data[index] + vertex_start);
                }
                break;
            }
            default:
                spdlog::error("Index component type {} not supported!", accessor.componentType);
                return;
            }

            ModelPrimitive new_primitive{};
            new_primitive.first_index = first_index;
            new_primitive.index_count = index_count;
            new_primitive.material_index = primitive.material;
            new_node.mesh.push_back(new_primitive);
        }
    }

    if (parent) {
        parent->children.push_back(new_node);
    } else {
        m_nodes.push_back(new_node);
    }
}

void Model::load_nodes() {
    spdlog::info("Loading {} glTF2 model scenes", m_model.scenes.size());

    // Preallocate memory for the model model.
    m_scenes.reserve(m_model.scenes.size());

    std::size_t scene_index = 0;

    for (const auto &scene : m_model.scenes) {
        for (const auto &node : scene.nodes) {
            auto vertices = m_scenes[scene_index].get_vertices();
            auto indices = m_scenes[scene_index].get_indices();
            load_node(m_model.nodes[node], nullptr, vertices, indices);
        }

        scene_index++;
    }
}

} // namespace inexor::vulkan_renderer::gltf2
