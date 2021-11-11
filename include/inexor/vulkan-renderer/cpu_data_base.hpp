#pragma once

#include <spdlog/spdlog.h>

#include <vector>

namespace inexor::vulkan_renderer {

template <typename VertexType, typename IndexType>
class CpuDataBase {
protected:
    std::vector<VertexType> m_vertices{};
    std::vector<IndexType> m_indices{};

    virtual void generate_vertices() = 0;

    void generate_indices() {
        auto old_vertices = std::move(m_vertices);

        // The C++0x standard guarantees that the container that was std::moved is in a valid but unspecified state.
        // We are allowed to reuse it after we bring it back into a specified state by clearing the vector.
        m_vertices.clear();

        m_indices.clear();
        m_indices.reserve(old_vertices.size());

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

} // namespace inexor::vulkan_renderer