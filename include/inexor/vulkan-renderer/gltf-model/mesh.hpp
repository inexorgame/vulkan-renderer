#pragma once

#include "inexor/vulkan-renderer/gltf-model/bounding_box.hpp"
#include "inexor/vulkan-renderer/gltf-model/primitive.hpp"
#include "inexor/vulkan-renderer/gltf-model/uniform_buffer.hpp"
#include "inexor/vulkan-renderer/uniform_buffer.hpp"
#include "inexor/vulkan-renderer/uniform_buffer_manager.hpp"

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::gltf_model {

struct Mesh {

    std::vector<std::shared_ptr<Primitive>> primitives;

    BoundingBox bb;

    BoundingBox aabb;

    std::shared_ptr<UniformBuffer> uniform_buffer;

    StandardUniformBufferBlock uniform_block;

    Mesh() = default;

    ~Mesh() = default;

    // TODO: Refactor!

    /// @brief Sets the model matrix.
    /// @param mat [in] The input matrix.
    void set_matrix(const glm::mat4 &mat) { uniform_block.matrix = mat; }

    /// @brief Sets the bounding box of the model.
    /// @param min [in] The minimum vector (edge of the bounding box)
    /// @param max [in] The maximum vector (edge of the bounding box)
    void set_bounding_box(glm::vec3 min, glm::vec3 max) {
        bb.min = min;
        bb.max = max;
        bb.valid = true;
    }
};

} // namespace inexor::vulkan_renderer::gltf_model
