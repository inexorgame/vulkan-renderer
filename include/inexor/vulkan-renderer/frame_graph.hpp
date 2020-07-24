#pragma once

// TODO(): Forward declare
#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/fence.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/pipeline_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <spdlog/spdlog.h>
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

enum class TextureUsage {
    INVALID,
    BACK_BUFFER,
    DEPTH_STENCIL_BUFFER,
    NORMAL,
};

class TextureResource : public RenderResource {
    friend FrameGraph;

private:
    VkFormat m_format{VK_FORMAT_UNDEFINED};
    TextureUsage m_usage{TextureUsage::INVALID};

public:
    explicit TextureResource(std::string &&name) : RenderResource(name) {}

    void set_format(VkFormat format) {
        m_format = format;
    }

    void set_usage(TextureUsage usage) {
        m_usage = usage;
    }
};

/// @brief A single render stage in the frame graph
/// @note Not to be confused with a vulkan render pass!
class RenderStage {
    friend FrameGraph;

private:
    const std::string m_name;
    std::vector<const RenderResource *> m_writes;
    std::vector<const RenderResource *> m_reads;

    std::vector<VkDescriptorSetLayout> m_descriptor_layouts;
    std::function<void(const class PhysicalStage *, const wrapper::CommandBuffer &)> m_on_record;

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

    void set_on_record(std::function<void(const class PhysicalStage *, const wrapper::CommandBuffer &)> on_record) {
        m_on_record = std::move(on_record);
    }
};

class GraphicsStage : public RenderStage {
    friend FrameGraph;

private:
    std::vector<VkPipelineShaderStageCreateInfo> m_shaders;
    std::vector<VkVertexInputAttributeDescription> m_attribute_bindings;
    std::vector<VkVertexInputBindingDescription> m_vertex_bindings;

public:
    explicit GraphicsStage(std::string &&name) : RenderStage(name) {}
    GraphicsStage(const GraphicsStage &) = delete;
    GraphicsStage(GraphicsStage &&) = delete;
    ~GraphicsStage() override = default;

    GraphicsStage &operator=(const GraphicsStage &) = delete;
    GraphicsStage &operator=(GraphicsStage &&) = delete;

    void uses_shader(const wrapper::Shader &shader);

    void add_attribute_binding(VkVertexInputAttributeDescription attribute_binding) {
        m_attribute_bindings.push_back(attribute_binding);
    }

    void add_vertex_binding(VkVertexInputBindingDescription vertex_binding) {
        m_vertex_bindings.push_back(vertex_binding);
    }
};

// TODO(): Add wrapper::Allocation that can be made by doing device->make<Allocation>(...);
class PhysicalResource {
    friend FrameGraph;

protected:
    // TODO(): Add OOP device functions (see above todo) and only store a wrapper::Device here
    VmaAllocator m_allocator;
    VkDevice m_device;
    VmaAllocation m_allocation{VK_NULL_HANDLE};

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
    VkImage m_image{VK_NULL_HANDLE};
    VkImageView m_image_view{VK_NULL_HANDLE};

public:
    PhysicalImage(VmaAllocator allocator, VkDevice device) : PhysicalResource(allocator, device) {}
    PhysicalImage(const PhysicalImage &) = delete;
    PhysicalImage(PhysicalImage &&) = delete;
    ~PhysicalImage() override;

    PhysicalImage &operator=(const PhysicalImage &) = delete;
    PhysicalImage &operator=(PhysicalImage &&) = delete;
};

class PhysicalBackBuffer : public PhysicalResource {
    friend FrameGraph;

private:
    const wrapper::Swapchain &m_swapchain;

public:
    PhysicalBackBuffer(VmaAllocator allocator, VkDevice device, const wrapper::Swapchain &swapchain)
        : PhysicalResource(allocator, device), m_swapchain(swapchain) {}
    PhysicalBackBuffer(const PhysicalBackBuffer &) = delete;
    PhysicalBackBuffer(PhysicalBackBuffer &&) = delete;
    ~PhysicalBackBuffer() override = default;

    PhysicalBackBuffer &operator=(const PhysicalBackBuffer &) = delete;
    PhysicalBackBuffer &operator=(PhysicalBackBuffer &&) = delete;
};

class PhysicalStage {
    friend FrameGraph;

private:
    std::vector<wrapper::CommandBuffer> m_command_buffers;
    VkDevice m_device;
    VkPipeline m_pipeline{VK_NULL_HANDLE};
    std::unique_ptr<wrapper::PipelineLayout> m_pipeline_layout;

protected:
    [[nodiscard]] VkDevice device() const {
        return m_device;
    }

public:
    explicit PhysicalStage(VkDevice device) : m_device(device) {}
    PhysicalStage(const PhysicalStage &) = delete;
    PhysicalStage(PhysicalStage &&) = delete;
    virtual ~PhysicalStage();

    PhysicalStage &operator=(const PhysicalStage &) = delete;
    PhysicalStage &operator=(PhysicalStage &&) = delete;

    [[nodiscard]] VkPipelineLayout pipeline_layout() const {
        return m_pipeline_layout->get();
    }
};

class PhysicalGraphicsStage : public PhysicalStage {
    friend FrameGraph;

private:
    VkRenderPass m_render_pass{VK_NULL_HANDLE};
    std::vector<wrapper::Framebuffer> m_framebuffers;

public:
    explicit PhysicalGraphicsStage(VkDevice device) : PhysicalStage(device) {}
    PhysicalGraphicsStage(const PhysicalGraphicsStage &) = delete;
    PhysicalGraphicsStage(PhysicalGraphicsStage &&) = delete;
    ~PhysicalGraphicsStage() override;

    PhysicalGraphicsStage &operator=(const PhysicalGraphicsStage &) = delete;
    PhysicalGraphicsStage &operator=(PhysicalGraphicsStage &&) = delete;
};

class FrameGraph {
private:
    VkDevice m_device;
    VkCommandPool m_command_pool;
    VmaAllocator m_allocator;
    const wrapper::Swapchain &m_swapchain;
    std::shared_ptr<spdlog::logger> m_log = spdlog::default_logger()->clone("frame-graph");

    // NOTE: unique_ptr must be used as Render* is just the base class
    std::vector<std::unique_ptr<RenderResource>> m_resources;
    std::vector<std::unique_ptr<RenderStage>> m_stages;

    // Stage execution order
    std::vector<RenderStage *> m_stage_stack;

    // Resource to physical resource map
    std::unordered_map<const RenderResource *, std::unique_ptr<PhysicalResource>> m_resource_map;

    // Stage to physical stage map
    std::unordered_map<const RenderStage *, std::unique_ptr<PhysicalStage>> m_stage_map;

    // Helper function used to create a physical resource during frame graph compilation
    // TODO(): Use concepts when we switch to C++ 20
    template <typename T, typename... Args, std::enable_if_t<std::is_base_of_v<PhysicalResource, T>, int> = 0>
    T *create(const RenderResource *resource, Args &&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        auto *ret = ptr.get();
        m_resource_map.emplace(resource, std::move(ptr));
        return ret;
    }

    // Helper function used to create a physical stage during frame graph compilation
    // TODO(): Use concepts when we switch to C++ 20
    template <typename T, typename... Args, std::enable_if_t<std::is_base_of_v<PhysicalStage, T>, int> = 0>
    T *create(const RenderStage *stage, Args &&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        auto *ret = ptr.get();
        m_stage_map.emplace(stage, std::move(ptr));
        return ret;
    }

    // Physical resources
    void build_image(const TextureResource *, PhysicalImage *, VmaAllocationCreateInfo *);
    void build_image_view(const TextureResource *, PhysicalImage *);

    // Physical stages
    void build_render_pass(const GraphicsStage *, PhysicalGraphicsStage *);
    void build_graphics_pipeline(const GraphicsStage *, PhysicalGraphicsStage *);
    void alloc_command_buffers(const RenderStage *, PhysicalStage *);
    void record_command_buffers(const RenderStage *, PhysicalStage *);

public:
    FrameGraph(VkDevice device, VkCommandPool command_pool, VmaAllocator allocator, const wrapper::Swapchain &swapchain)
        : m_device(device), m_command_pool(command_pool), m_allocator(allocator), m_swapchain(swapchain) {}

    template <typename T, typename... Args>
    T &add(Args &&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        auto &ret = *ptr;
        if constexpr (std::is_base_of_v<RenderResource, T>) {
            m_resources.push_back(std::move(ptr));
        } else if constexpr (std::is_base_of_v<RenderStage, T>) {
            m_stages.push_back(std::move(ptr));
        } else {
            static_assert(!std::is_same_v<T, T>, "T must be a RenderResource or RenderStage");
        }
        return ret;
    }

    void compile(const RenderResource &target);
    void render(int image_index, VkSemaphore signal_semaphore, VkSemaphore wait_semaphore,
                VkQueue graphics_queue) const;
};

} // namespace inexor::vulkan_renderer
