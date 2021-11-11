#pragma once

#include "inexor/vulkan-renderer/cpu_data_base.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"

#include <unordered_map>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::world {

template <typename VertexType, typename IndexType = std::uint32_t>
class OctreeCpuData : public CpuDataBase<VertexType, IndexType> {
private:
    const Cube &m_cube;

    void generate_vertices() override {
        CpuDataBase<VertexType, IndexType>::m_vertices.clear();

        for (const auto &polygons : m_cube.polygons(true)) {
            for (const auto &triangle : *polygons) {
                for (const auto &vertex : triangle) {
                    const glm::vec3 color = {
                        // TODO: Use Iceflower's approach to random numbers and get rid of rand()!
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    };
                    CpuDataBase<VertexType, IndexType>::m_vertices.emplace_back(vertex, color);
                }
            }
        }
    }

public:
    // TODO: accept parameter as const reference or shared_ptr?
    OctreeCpuData(const Cube &cube) : m_cube(cube) {
        generate_vertices();
        CpuDataBase<VertexType, IndexType>::generate_indices();
    }
};

} // namespace inexor::vulkan_renderer::world
