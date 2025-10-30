#include <inexor/vulkan-renderer/octree/cube.hpp>

#include <gtest/gtest.h>

namespace {
using namespace inexor::vulkan_renderer::octree;

TEST(Cube, neighbor) {
    std::shared_ptr<Cube> root = std::make_shared<Cube>(2.0f, glm::vec3{0, -1, -1});
    root->set_type(Cube::Type::OCTANT);

    for (const auto &child : root->children()) {
        child->set_type(Cube::Type::OCTANT);

        for (const auto &grandchild : child->children()) {
            grandchild->set_type(Cube::Type::OCTANT);
        }
    }

    EXPECT_EQ(root->children()[1]->neighbor(Cube::NeighborAxis::Y, Cube::NeighborDirection::POSITIVE),
              root->children()[3]);
    EXPECT_NE(root->children()[1]->neighbor(Cube::NeighborAxis::Y, Cube::NeighborDirection::POSITIVE),
              root->children()[0]);
    EXPECT_EQ(root->children()[1]->children()[2]->neighbor(Cube::NeighborAxis::Y, Cube::NeighborDirection::POSITIVE),
              root->children()[3]->children()[0]);
    EXPECT_EQ(root->children()[1]->children()[2]->neighbor(Cube::NeighborAxis::Z, Cube::NeighborDirection::NEGATIVE),
              root->children()[0]->children()[3]);
}

} // namespace
