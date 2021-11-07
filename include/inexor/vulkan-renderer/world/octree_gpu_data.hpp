#pragma once

#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace inexor::vulkan_renderer::world {

// TODO: Implement octree gpu data generator?
// TODO: Maybe implement a RenderingData template base class?
template <typename VertexType, typename IndexType, typename UniformBufferObjectType>
class OctreeGPUData {
private:
    std::shared_ptr<world::Cube> m_cube;

    std::vector<VertexType> m_vertices{};
    std::vector<IndexType> m_indices{};

    std::unique_ptr<wrapper::DescriptorPool> m_descriptor_pool;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;
    std::unique_ptr<wrapper::UniformBuffer> m_uniform_buffer;

    BufferResource *m_index_buffer{nullptr};
    BufferResource *m_vertex_buffer{nullptr};

    /// @brief Generate the vertices of the octree.
    void generate_vertices() {
        m_vertices.clear();

        for (const auto &polygons : m_cube->polygons(true)) {
            for (const auto &triangle : *polygons) {
                for (const auto &vertex : triangle) {
                    const glm::vec3 color = {
                        // TODO: Use Iceflower's approach to random numbers and get rid of rand()!
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    };
                    m_vertices.emplace_back(vertex, color);
                }
            }
        }
    }

    /// @brief Generate the indices of the octree.
    void generate_indices() {
        auto old_vertices = std::move(m_vertices);

        // The C++0x standard guarantees that the container that was std::moved is in a valid but unspecified state.
        // We are allowed to reuse it after we bring it back into a specified state by clearing the vector.
        m_vertices.clear();

        m_indices.clear();

        std::unordered_map<VertexType, IndexType> vertex_map;

        for (auto &vertex : old_vertices) {
            // TODO: Use std::unordered_map::contains() when we switch to C++ 20.
            if (vertex_map.count(vertex) == 0) {
                assert(vertex_map.size() < std::numeric_limits<std::uint32_t>::max() && "Octree too big!");
                vertex_map.emplace(vertex, static_cast<std::uint32_t>(vertex_map.size()));
                m_vertices.push_back(vertex);
            }

            m_indices.push_back(vertex_map.at(vertex));
        }

        spdlog::trace("Reduced octree by {} vertices (from {} to {})", old_vertices.size() - m_vertices.size(),
                      old_vertices.size(), m_vertices.size());

        spdlog::trace("Total indices {} ", m_vertices.size());
    }

    void setup_rendering_resources(RenderGraph *render_graph) {
        m_vertex_buffer = render_graph->add<BufferResource>("octree vertices", BufferUsage::VERTEX_BUFFER);

        m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexType, position))
            ->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexType, color))
            ->set_element_size<VertexType>()
            ->upload_data(m_vertices);

        m_index_buffer = render_graph->add<BufferResource>("octree indices", BufferUsage::INDEX_BUFFER);
        m_index_buffer->upload_data(m_indices);

        const std::vector<VkDescriptorPoolSize> pool_sizes{{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};

        m_descriptor_pool =
            std::make_unique<wrapper::DescriptorPool>(render_graph->device_wrapper(), pool_sizes, "octree");

        // Create an instance of the resource descriptor builder.
        // This allows us to make resource descriptors with the help of a builder pattern.
        wrapper::DescriptorBuilder descriptor_builder(render_graph->device_wrapper(),
                                                      m_descriptor_pool->descriptor_pool());

        // TODO: Make UniformBuffer a templated type!
        m_uniform_buffer = std::make_unique<wrapper::UniformBuffer>(render_graph->device_wrapper(), "octree",
                                                                    sizeof(UniformBufferObjectType));

        m_descriptor =
            descriptor_builder.add_uniform_buffer<UniformBufferObjectType>(m_uniform_buffer->buffer()).build("octree");
    }

public:
    OctreeGPUData(RenderGraph *render_graph, const std::shared_ptr<world::Cube> cube) : m_cube(cube) {
        generate_vertices();
        generate_indices();
        setup_rendering_resources(render_graph);
    }

    OctreeGPUData() = delete;
    OctreeGPUData(const OctreeGPUData &) = delete;

    // Since this is a template class, we can't put the move assignment constructor in the cpp file.
    OctreeGPUData(OctreeGPUData &&other) noexcept {
        m_cube = std::move(other.m_cube);
        m_vertices = std::move(other.m_vertices);
        m_indices = std::move(other.m_indices);
        m_descriptor_pool = std::move(other.m_descriptor_pool);
        m_descriptor = std::move(other.m_descriptor);
        m_uniform_buffer = std::move(other.m_uniform_buffer);
        m_vertex_buffer = std::exchange(other.m_vertex_buffer, nullptr);
        m_index_buffer = std::exchange(other.m_index_buffer, nullptr);
    }

    ~OctreeGPUData() = default;

    OctreeGPUData &operator=(const OctreeGPUData &) = delete;
    OctreeGPUData &operator=(OctreeGPUData &&) = default;

    /// @brief Updates the vertices and indices of the new octree.
    /// @param new_cube The new cube to process vertices and indices from
    void update_octree(const std::shared_ptr<Cube> new_cube) {
        m_cube.reset();
        m_cube = new_cube;

        spdlog::trace("Regenerating octree vertices");
        generate_vertices();

        spdlog::trace("regenerating octree indices");
        generate_indices();

        m_vertex_buffer->upload_data(m_vertices);
        m_index_buffer->upload_data(m_indices);
    }

    /// @brief Update the otree uniform buffer.
    /// @param data The new uniform buffer data of template type UniformBufferObjectType
    void update_ubo(const UniformBufferObjectType *data) {
        m_uniform_buffer->update<UniformBufferObjectType>(data);
    }

    /// @note This might return an empty vector.
    [[nodiscard]] const auto &vertices() const {
        return m_vertices;
    }

    /// @note This might return an empty vector.
    [[nodiscard]] const auto &indices() const {
        return m_indices;
    };

    [[nodiscard]] auto vertex_count() const {
        return m_vertices.size();
    }

    [[nodiscard]] auto index_count() const {
        return m_indices.size();
    }

    [[nodiscard]] auto *vertex_buffer() const {
        return m_vertex_buffer;
    }

    [[nodiscard]] auto *index_buffer() const {
        return m_index_buffer;
    }

    [[nodiscard]] VkDescriptorSet descriptor_set() const {
        return m_descriptor->descriptor_set();
    }

    [[nodiscard]] VkDescriptorSetLayout descriptor_set_layout() const {
        return m_descriptor->descriptor_set_layout();
    }
};

} // namespace inexor::vulkan_renderer::world
