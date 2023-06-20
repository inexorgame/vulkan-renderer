#pragma once

#include <volk.h>

#include <functional>

namespace inexor::vulkan_renderer::render_graph {

// Forward declaration
class RenderGraph;

/// A wrapper class for push constant ranges
class PushConstantRangeResource {
    friend RenderGraph;

private:
    VkPushConstantRange m_push_constant;
    std::function<void()> m_on_update{[]() {}};
    const void *m_push_constant_data{nullptr};

public:
    /// Default constructor
    /// @param push_constant The push constant
    /// @param push_constant_data The data of the push constant
    /// @param on_update The update function of the push constant
    PushConstantRangeResource(VkPushConstantRange push_constant, const void *push_constant_data,
                              std::function<void()> on_update);

    PushConstantRangeResource(const PushConstantRangeResource &) = delete;
    PushConstantRangeResource(PushConstantRangeResource &&other) noexcept;
    ~PushConstantRangeResource() = default;

    PushConstantRangeResource &operator=(const PushConstantRangeResource &) = delete;
    PushConstantRangeResource &operator=(PushConstantRangeResource &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
