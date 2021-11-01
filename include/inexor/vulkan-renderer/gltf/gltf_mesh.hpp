#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_primitive.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <glm/mat4x4.hpp>

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::gltf {

const std::uint32_t MAX_NUM_JOINTS{128u};

struct ModelMesh {

    ModelMesh(const wrapper::Device &device, glm::mat4 matrix);

    void set_bounding_box(glm::vec3 min, glm::vec3 max);

    std::unique_ptr<wrapper::UniformBuffer> ubo;

    std::vector<ModelPrimitive> primitives;

    BoundingBox bb;
    BoundingBox aabb;

    struct UniformBlock {
        glm::mat4 matrix;
        glm::mat4 jointMatrix[128]{};
        float jointcount{0};
    } uniformBlock;
};

} // namespace inexor::vulkan_renderer::gltf
