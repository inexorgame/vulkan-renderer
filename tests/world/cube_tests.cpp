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

    EXPECT_EQ(root->children()[1]->neighbor(Cube::Axis::Y, Cube::NeighborDirection::POSITIVE), root->children()[3]);
    EXPECT_NE(root->children()[1]->neighbor(Cube::Axis::Y, Cube::NeighborDirection::POSITIVE), root->children()[0]);
    EXPECT_EQ(root->children()[1]->children()[2]->neighbor(Cube::Axis::Y, Cube::NeighborDirection::POSITIVE),
              root->children()[3]->children()[0]);
    EXPECT_EQ(root->children()[1]->children()[2]->neighbor(Cube::Axis::Z, Cube::NeighborDirection::NEGATIVE),
              root->children()[0]->children()[3]);
}

TEST(Cube, simplify) {
    auto root = std::make_shared<Cube>(2.0f, glm::vec3{0, 0, 0});
    root->set_type(Cube::Type::OCTANT);

    // Setting all 8 children to SOLID should simplify root to SOLID.
    for (const auto &child : root->children()) {
        child->set_type(Cube::Type::SOLID);
    }
    EXPECT_EQ(root->type(), Cube::Type::SOLID);

    // Octant with mixed children does not simplify.
    root->set_type(Cube::Type::OCTANT);
    for (std::size_t i = 0; i < 7; ++i) {
        root->children()[i]->set_type(Cube::Type::SOLID);
    }
    EXPECT_EQ(root->type(), Cube::Type::OCTANT);
}

} // namespace
