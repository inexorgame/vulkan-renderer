#pragma once

#include "inexor/vulkan-renderer/world/cube.hpp"

#include <spdlog/spdlog.h>

#include <unordered_map>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::world {

template <typename VertexType, typename IndexType = std::uint32_t>
class OctreeCPUData {
private:
    std::vector<VertexType> m_vertices{};
    std::vector<IndexType> m_indices{};

    void generate_vertices(const Cube &cube) {
        m_vertices.clear();

        for (const auto &polygons : cube.polygons(true)) {
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

public:
    // TODO: accept parameter as const reference or shared_ptr?
    OctreeCPUData(const Cube &cube) {
        generate_vertices(cube);
        generate_indices();
    }

    [[nodiscard]] const auto &vertices() const {
        return m_vertices;
    }

    [[nodiscard]] const auto &indices() const {
        return m_indices;
    };

    [[nodiscard]] auto vertex_count() const {
        return m_vertices.size();
    }

    [[nodiscard]] auto index_count() const {
        return m_indices.size();
    }
};

} // namespace inexor::vulkan_renderer::world
