#pragma once

// TODO(): Forward declare
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/pipeline_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// TODO(): Compute stages
// TODO(): Uniform buffers

namespace inexor::vulkan_renderer {

class FrameGraph;

class RenderResource {
    friend FrameGraph;

private:
    const std::string m_name;

protected:
    explicit RenderResource(std::string name) : m_name(std::move(name)) {}

public:
    RenderResource(const RenderResource &) = delete;
    RenderResource(RenderResource &&) = delete;
    virtual ~RenderResource() = default;

    RenderResource &operator=(const RenderResource &) = delete;
    RenderResource &operator=(RenderResource &&) = delete;
};

class BufferResource : public RenderResource {
    friend FrameGraph;

private:
    VkBufferUsageFlags m_usage;

public:
    explicit BufferResource(std::string &&name) : RenderResource(name) {}

    void set_usage(VkBufferUsageFlags usage) {
        m_usage = usage;
    }
};

enum class TextureUsage {
    BACK_BUFFER,
    DEPTH_BUFFER,
    NORMAL,
};

class TextureResource : public RenderResource {
    friend FrameGraph;

private:
    VkFormat m_format;
    TextureUsage m_usage;

public:
    explicit TextureResource(std::string &&name) : RenderResource(name) {}

    void set_format(VkFormat format) {
        m_format = format;
    }

    void set_usage(TextureUsage usage) {
        m_usage = usage;
    }
};

// TODO(): Add wrapper::Allocation that can be made by doing device->make<Allocation>(...);
class PhysicalResource {
    friend FrameGraph;

protected:
    // TODO(): Merge allocator with device (see above todo)
    // TODO(): Protected fields :(
    VmaAllocator m_allocator;
    VkDevice m_device;
    VmaAllocation m_allocation;

    PhysicalResource(VmaAllocator allocator, VkDevice device) : m_allocator(allocator), m_device(device) {}

public:
    PhysicalResource(const PhysicalResource &) = delete;
    PhysicalResource(PhysicalResource &&) = delete;
    virtual ~PhysicalResource() = default;

    PhysicalResource &operator=(const PhysicalResource &) = delete;
    PhysicalResource &operator=(PhysicalResource &&) = delete;
};

class PhysicalImage : public PhysicalResource {
    friend FrameGraph;

private:
    VkImage m_image;
    VkImageView m_image_view;

public:
    PhysicalImage(VmaAllocator allocator, VkDevice device) : PhysicalResource(allocator, device) {}
    ~PhysicalImage() override;
};

class PhysicalBackBuffer : public PhysicalImage {
    friend FrameGraph;

private:
    std::unique_ptr<wrapper::Framebuffer> m_framebuffer;

public:
    PhysicalBackBuffer(VmaAllocator allocator, VkDevice device) : PhysicalImage(allocator, device) {}
    ~PhysicalBackBuffer() override = default;
};

/// @brief A single render stage in the frame graph.
/// @note Not to be confused with a vulkan render pass!
class RenderStage {
    friend FrameGraph;

private:
    const std::string m_name;
    std::vector<const RenderResource *> m_writes;
    std::vector<const RenderResource *> m_reads;

    std::vector<VkDescriptorSetLayout> m_descriptor_layouts;
    std::function<void(VkCommandBuffer)> m_on_record;

    // TODO(): Nice pipeline and pipeline layout wrappers
    std::vector<VkCommandBuffer> m_command_buffers;
    VkPipeline m_pipeline;
    std::unique_ptr<wrapper::PipelineLayout> m_pipeline_layout;

protected:
    explicit RenderStage(std::string name) : m_name(std::move(name)) {}

public:
    RenderStage(const RenderStage &) = delete;
    RenderStage(RenderStage &&) = delete;
    virtual ~RenderStage() = default;

    RenderStage &operator=(const RenderStage &) = delete;
    RenderStage &operator=(RenderStage &&) = delete;

    /// @brief Specifies that this stage writes to @param resource
    void writes_to(const RenderResource &resource);

    /// @brief Specifies that this stage reads from @param resource
    void reads_from(const RenderResource &resource);

    void add_descriptor_layout(VkDescriptorSetLayout layout) {
        m_descriptor_layouts.push_back(layout);
    }

    void set_on_record(std::function<void(VkCommandBuffer)> on_record) {
        m_on_record = std::move(on_record);
    }

    VkPipelineLayout pipeline_layout() const {
        return m_pipeline_layout->get();
    }
};

class GraphicsStage : public RenderStage {
    friend FrameGraph;

private:
    VkClearValue m_clear_colour;
    VkRenderPass m_render_pass;

    std::vector<VkPipelineShaderStageCreateInfo> m_shaders;
    std::vector<VkVertexInputAttributeDescription> m_attribute_bindings;
    std::vector<VkVertexInputBindingDescription> m_vertex_bindings;

public:
    explicit GraphicsStage(std::string &&name) : RenderStage(name) {}

    void set_clear_colour(VkClearColorValue colour, VkClearDepthStencilValue depth_stencil) {
        m_clear_colour.color = colour;
        m_clear_colour.depthStencil = depth_stencil;
    }

    void uses_shader(const wrapper::Shader &shader);

    void add_attribute_binding(VkVertexInputAttributeDescription attribute_binding) {
        m_attribute_bindings.push_back(attribute_binding);
    }

    void add_vertex_binding(VkVertexInputBindingDescription vertex_binding) {
        m_vertex_bindings.push_back(vertex_binding);
    }
};

class FrameGraph {
private:
    // NOTE: unique_ptr must be used as Render* is just the base class
    std::vector<std::unique_ptr<RenderResource>> m_resources;
    std::vector<std::unique_ptr<RenderStage>> m_stages;

    // Stage execution order
    std::vector<RenderStage *> m_stage_stack;

    // Resource to physical resource map
    std::unordered_map<const RenderResource *, std::unique_ptr<PhysicalResource>> m_resource_map;

    // TODO(): Use concepts when we switch to C++ 20
    template <typename T, typename... Args, std::enable_if_t<std::is_base_of_v<PhysicalResource, T>, int> = 0>
    T &create_phys_resource(const RenderResource *resource, Args &&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        auto &ret = *ptr;
        m_resource_map.emplace(resource, std::move(ptr));
        return ret;
    }

public:
    FrameGraph();

    // TODO(): Use concepts when we switch to C++ 20
    template <typename T, typename... Args, std::enable_if_t<std::is_base_of_v<RenderResource, T>, int> = 0>
    T &add(Args &&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        auto &ret = *ptr;
        m_resources.push_back(std::move(ptr));
        return ret;
    }

    // TODO(): Use concepts when we switch to C++ 20
    template <typename T, typename... Args, std::enable_if_t<std::is_base_of_v<RenderStage, T>, int> = 0>
    T &add(Args &&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        auto &ret = *ptr;
        m_stages.push_back(std::move(ptr));
        return ret;
    }

    void compile(const RenderResource &target, VkDevice device, VkCommandPool command_pool, VmaAllocator allocator,
                 const wrapper::Swapchain &swapchain);
};

} // namespace inexor::vulkan_renderer
