#pragma once
#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_updater.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/render_pass.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

// TODO: Compute stages.

// Forward declarations
namespace inexor::vulkan_renderer::wrapper {
class CommandBuffer;
class Shader;
}; // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer {

// Forward declarations
class PhysicalResource;
class PhysicalStage;
class RenderGraph;

/// @brief Base class of all render graph objects (resources and stages).
/// @note This is just for internal use.
struct RenderGraphObject {
    RenderGraphObject() = default;
    RenderGraphObject(const RenderGraphObject &) = delete;
    RenderGraphObject(RenderGraphObject &&) = delete;
    virtual ~RenderGraphObject() = default;

    RenderGraphObject &operator=(const RenderGraphObject &) = delete;
    RenderGraphObject &operator=(RenderGraphObject &&) = delete;

    /// @brief Casts this object to type `T`
    /// @return The object as type `T` or `nullptr` if the cast failed
    template <typename T>
    [[nodiscard]] T *as() {
        return dynamic_cast<T *>(this);
    }

    /// @copydoc as
    template <typename T>
    [[nodiscard]] const T *as() const {
        return dynamic_cast<const T *>(this);
    }
};

/// @brief A single resource in the render graph.
/// @note May become multiple physical (vulkan) resources during render graph compilation.
class RenderResource : public RenderGraphObject {
    friend RenderGraph;

private:
    const std::string m_name;
    std::shared_ptr<PhysicalResource> m_physical;

protected:
    explicit RenderResource(std::string name) : m_name(std::move(name)) {}

public:
    RenderResource(const RenderResource &) = delete;
    RenderResource(RenderResource &&) = delete;
    ~RenderResource() override = default;

    RenderResource &operator=(const RenderResource &) = delete;
    RenderResource &operator=(RenderResource &&) = delete;

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }
};

/// The internal buffer usage inside of rendergraph
enum class BufferUsage {
    INDEX_BUFFER,
    VERTEX_BUFFER,
    UNIFORM_BUFFER,
};

// Forward declaration
class PhysicalBuffer;

class BufferResource : public RenderResource {
    friend RenderGraph;

private:
    const BufferUsage m_usage;
    const void *m_data{nullptr};
    std::size_t m_data_size{0};
    std::function<void()> m_on_update{[]() {}};
    bool m_data_upload_needed{false};

    // This is required for uniform buffer updates
    // TODO: We should not do this in the future!
    PhysicalBuffer *m_physical_buffer{nullptr};
    std::size_t m_my_buffer_index{0};

public:
    BufferResource(
        std::string &&name, BufferUsage usage, std::function<void()> on_update = []() {})
        : RenderResource(name), m_usage(usage), m_on_update(std::move(on_update)) {}

    void announce_update(const void *data, std::size_t size) {
        m_data = data;
        m_data_size = size;
        m_data_upload_needed = true;
    }

    template <typename T>
    void announce_update(const T *data) {
        announce_update(data, sizeof(T));
    }

    template <typename T>
    void announce_update(const std::vector<T> &data) {
        announce_update(data.data(), sizeof(T) * data.size());
    }
};

enum class TextureUsage {
    /// @brief Specifies that this texture is the output of the render graph.
    // TODO: Refactor back buffer system more (remove need for BACK_BUFFER texture usage)
    BACK_BUFFER,

    /// @brief Specifies that this texture is a combined depth/stencil buffer.
    /// @note This may mean that this texture is completely GPU-sided and cannot be accessed by the CPU in any way.
    DEPTH_STENCIL_BUFFER,

    /// @brief Specifies that this texture isn't used for any special purpose.
    NORMAL,
};

class TextureResource : public RenderResource {
    friend RenderGraph;

private:
    const TextureUsage m_usage;
    VkFormat m_format{VK_FORMAT_UNDEFINED};

public:
    /// Default constructor
    /// @param usage The internal usage of the texture inside of rendergraph
    /// @param format The image format
    /// @param name The internal debug name of the texture
    TextureResource(TextureUsage usage, VkFormat format, std::string &&name)
        : RenderResource(name), m_usage(usage), m_format(format) {}
};

/// An external texture resoruce is a texture which already has gpu memory created for it
class ExternalTextureResource : public RenderResource {
    friend RenderGraph;

private:
    const wrapper::GpuTexture &m_texture;
    VkDescriptorImageInfo m_descriptor_image_info{};

public:
    ExternalTextureResource(const wrapper::GpuTexture &texture) : RenderResource(texture.name()), m_texture(texture) {}
};

class PushConstantResource {
    friend RenderGraph;

private:
    VkPushConstantRange m_push_constant;
    std::function<void()> m_on_update{[]() {}};
    const void *m_push_constant_data{nullptr};

public:
    PushConstantResource(const VkPushConstantRange push_constant, const void *push_constant_data,
                         std::function<void()> on_update)
        : m_push_constant(push_constant), m_push_constant_data(push_constant_data), m_on_update(std::move(on_update)) {}

    PushConstantResource(const PushConstantResource &) = delete;
    PushConstantResource(PushConstantResource &&other) noexcept : m_push_constant_data(other.m_push_constant_data) {
        m_push_constant = other.m_push_constant;
        m_on_update = std::move(other.m_on_update);
    };

    ~PushConstantResource() = default;

    PushConstantResource &operator=(const PushConstantResource &) = delete;
    PushConstantResource &operator=(PushConstantResource &&) = delete;
};

/// @brief A single render stage in the render graph.
/// @note Not to be confused with a vulkan render pass.
class RenderStage : public RenderGraphObject {
    friend RenderGraph;

private:
    const std::string m_name;
    std::unique_ptr<PhysicalStage> m_physical;
    std::vector<const RenderResource *> m_writes;
    // TODO: Incorporate gpu textures into rendergraph and make this a const* RenderResource* again!
    std::vector<std::pair<RenderResource *, std::optional<VkShaderStageFlags>>> m_reads;

    std::vector<PushConstantResource> m_push_constants;
    // We need to collect the push constant ranges into one vector
    std::vector<VkPushConstantRange> m_push_constant_ranges;

    std::function<void(void)> m_on_update{[]() {}};
    std::function<void(const wrapper::CommandBuffer &)> m_on_record{[](auto &) {}};

protected:
    explicit RenderStage(std::string name) : m_name(std::move(name)) {}

public:
    RenderStage(const RenderStage &) = delete;
    RenderStage(RenderStage &&) = delete;
    ~RenderStage() override = default;

    RenderStage &operator=(const RenderStage &) = delete;
    RenderStage &operator=(RenderStage &&) = delete;

    /// Specifies that this stage writes to `resource`
    RenderStage *writes_to(const RenderResource *resource);

    /// Specifies that this stage reads from `resource`
    // TODO: Incorporate gpu textures into rendergraph and make this a const* RenderResource* again!
    RenderStage *reads_from(RenderResource *resource);

    /// Specifies that this stage reads from `resource`
    // TODO: Incorporate gpu textures into rendergraph and make this a const* RenderResource* again!
    RenderStage *reads_from(RenderResource *resource, VkShaderStageFlags shader_stage);

    template <typename PushConstantRangeDataType>
    RenderStage *add_push_constant_range(
        const PushConstantRangeDataType *data, std::function<void()> on_update = []() {},
        const VkShaderStageFlags stage_flags = VK_SHADER_STAGE_VERTEX_BIT, const std::uint32_t offset = 0) {
        m_push_constants.emplace_back(
            VkPushConstantRange{
                .stageFlags = stage_flags,
                .offset = offset,
                .size = sizeof(PushConstantRangeDataType),
            },
            data, std::move(on_update));
        return this;
    }

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }

    RenderStage *set_on_update(std::function<void(void)> on_update) {
        m_on_update = std::move(on_update);
        return this;
    }

    /// Specifies a function that will be called during command buffer recording for this stage
    /// @details This function can be used to specify other vulkan commands during command buffer recording.
    /// The most common use for this is for draw commands.
    RenderStage *set_on_record(std::function<void(const wrapper::CommandBuffer &)> on_record) {
        m_on_record = std::move(on_record);
        return this;
    }
};

class GraphicsStage : public RenderStage {
    friend RenderGraph;

private:
    bool m_clears_screen{false};
    bool m_depth_test{false};
    bool m_depth_write{false};
    VkClearValue m_clear_value{};
    VkPipelineColorBlendAttachmentState m_color_blend_attachment{};

    std::vector<VkVertexInputBindingDescription> m_vertex_input_binding_descriptions;
    std::vector<VkVertexInputAttributeDescription> m_vertex_input_attribute_descriptions;
    VkPipelineVertexInputStateCreateInfo m_vertex_input_sci{
        wrapper::make_info<VkPipelineVertexInputStateCreateInfo>(),
    };
    VkPipelineInputAssemblyStateCreateInfo m_input_assembly_sci{
        wrapper::make_info<VkPipelineInputAssemblyStateCreateInfo>({
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        }),
    };
    VkPipelineTessellationStateCreateInfo m_tesselation_sci{
        wrapper::make_info<VkPipelineTessellationStateCreateInfo>(),
    };
    std::vector<VkViewport> m_viewports;
    std::vector<VkRect2D> m_scissors;
    VkPipelineViewportStateCreateInfo m_viewport_sci{
        wrapper::make_info<VkPipelineViewportStateCreateInfo>(),
    };
    VkPipelineRasterizationStateCreateInfo m_rasterization_sci{
        wrapper::make_info<VkPipelineRasterizationStateCreateInfo>({
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .lineWidth = 1.0f,
        }),
    };
    VkPipelineMultisampleStateCreateInfo m_multisample_sci{
        wrapper::make_info<VkPipelineMultisampleStateCreateInfo>({
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
        }),
    };
    VkPipelineDepthStencilStateCreateInfo m_depth_stencil_sci{
        wrapper::make_info<VkPipelineDepthStencilStateCreateInfo>(),
    };
    VkPipelineColorBlendStateCreateInfo m_color_blend_sci{
        wrapper::make_info<VkPipelineColorBlendStateCreateInfo>(),
    };
    std::vector<VkDynamicState> m_dynamic_states;
    VkPipelineDynamicStateCreateInfo m_dynamic_states_sci{
        wrapper::make_info<VkPipelineDynamicStateCreateInfo>(),
    };
    VkPipelineLayout m_pipeline_layout{VK_NULL_HANDLE};
    VkRenderPass m_render_pass{VK_NULL_HANDLE};
    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;

public:
    explicit GraphicsStage(std::string &&name) : RenderStage(name) {}
    GraphicsStage(const GraphicsStage &) = delete;
    GraphicsStage(GraphicsStage &&) = delete;
    ~GraphicsStage() override = default;

    GraphicsStage &operator=(const GraphicsStage &) = delete;
    GraphicsStage &operator=(GraphicsStage &&) = delete;

    // TODO: Use graphics pipeline builder directly and expose it to the front

    [[nodiscard]] VkGraphicsPipelineCreateInfo make_create_info();

    /// Add a shader stage
    /// @param shader The shader stage to add
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *add_shader(const VkPipelineShaderStageCreateInfo &shader);

    /// Add a shader stage
    /// @param shader The shader stage to add
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *add_shader(const wrapper::Shader &shader);

    /// Add a vertex input attribute description
    /// @param description The vertex input attribute description
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *add_vertex_input_attribute(const VkVertexInputAttributeDescription &description);

    /// Add a vertex input binding description
    /// @param description The vertex input binding descriptions
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *add_vertex_input_binding(const VkVertexInputBindingDescription &description);

    /// Specifies that this stage should clear the screen before rendering
    [[nodiscard]] GraphicsStage *set_clears_screen(bool clears_screen) {
        m_clears_screen = clears_screen;
        return this;
    }

    [[nodiscard]] GraphicsStage *set_clears_screen_color(const VkClearValue clear_value) {
        m_clear_value = clear_value;
        return this;
    }

    /// Specifies the depth options for this stage
    /// @param depth_test Whether depth testing should be performed
    /// @param depth_write Whether depth writing should be performed
    [[nodiscard]] GraphicsStage *set_depth_options(bool depth_test, bool depth_write) {
        m_depth_test = depth_test;
        m_depth_write = depth_write;
        return this;
    }

    /// Set the blend attachment for this stage
    /// @param blend_attachment The blend attachment
    [[nodiscard]] GraphicsStage *set_blend_attachment(VkPipelineColorBlendAttachmentState blend_attachment) {
        m_color_blend_attachment = blend_attachment;
        return this;
    }

    /// Add a color blend attachment
    /// @param attachment The color blend attachment
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *add_color_blend_attachment(const VkPipelineColorBlendAttachmentState &attachment);

    /// Set the color blend manually
    /// @param color_blend The color blend
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_color_blend(const VkPipelineColorBlendStateCreateInfo &color_blend);

    /// Set all color blend attachments manually
    /// @note You should prefer to use ``add_color_blend_attachment`` instead
    /// @param attachments The color blend attachments
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *
    set_color_blend_attachments(const std::vector<VkPipelineColorBlendAttachmentState> &attachments);

    /// Enable or disable culling
    /// @warning Disabling culling will have a significant performance impact
    /// @param culling_enabled ``true`` if culling is enabled
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_culling_mode(VkBool32 culling_enabled);

    /// Set the depth stencil
    /// @param depth_stencil The depth stencil
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_depth_stencil(const VkPipelineDepthStencilStateCreateInfo &depth_stencil);

    /// Set the dynamic states
    /// @param dynamic_states The dynamic states
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_dynamic_states(const std::vector<VkDynamicState> &dynamic_states);

    /// Set the input assembly state create info
    /// @note If you just want to set the triangle topology, call ``set_triangle_topology`` instead, because this is the
    /// most powerful method of this method in case you really need to overwrite it
    /// @param input_assembly The pipeline input state create info
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_input_assembly(const VkPipelineInputAssemblyStateCreateInfo &input_assembly);

    /// Set the line width of rasterization
    /// @param line_width The line width of rasterization
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_line_width(float width);

    /// Set the most important MSAA settings
    /// @param sample_count The number of samples used in rasterization
    /// @param min_sample_shading A minimum fraction of sample shading
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_multisampling(VkSampleCountFlagBits sample_count, float min_sample_shading);

    /// Store the pipeline layout
    /// @param layout The pipeline layout
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_pipeline_layout(VkPipelineLayout layout);

    /// Set the triangle topology
    /// @param topology the primitive topology
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_primitive_topology(VkPrimitiveTopology topology);

    /// Set the rasterization state of the graphics pipeline manually
    /// @param rasterization The rasterization state
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_rasterization(const VkPipelineRasterizationStateCreateInfo &rasterization);

    /// Set the render pass
    /// @param render_pass The render pass
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_render_pass(VkRenderPass render_pass);

    /// Set the scissor data in VkPipelineViewportStateCreateInfo
    /// There is another method called set_scissors in case multiple scissors will be used
    /// @param scissors The scissors in in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_scissor(const VkRect2D &scissor);

    /// Set the scissor data in VkPipelineViewportStateCreateInfo (convert VkExtent2D to VkRect2D)
    /// @param extent The extent of the scissor
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_scissor(const VkExtent2D &extent);

    /// Set the viewport data in VkPipelineViewportStateCreateInfo
    /// There is another method called set_scissors in case multiple scissors will be used
    /// @param scissor The scissor in in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_scissors(const std::vector<VkRect2D> &scissors);

    /// Set the shader stage
    /// @param shader_stages The shader stages
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_shaders(const std::vector<VkPipelineShaderStageCreateInfo> &shaders);

    /// Set the tesselation control point count
    /// @param control_point_count The tesselation control point count
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_tesselation_control_point_count(std::uint32_t control_point_count);

    /// Set the vertex input attribute descriptions manually
    /// You should prefer to use ``add_vertex_input_attribute`` instead
    /// @param descriptions The vertex input attribute descriptions
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *
    set_vertex_input_attribute_descriptions(const std::vector<VkVertexInputAttributeDescription> &descriptions);

    /// Set the vertex input binding descriptions manually
    /// You should prefer to use ``add_vertex_input_binding`` instead
    /// @param descriptions The vertex input binding descriptions
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *
    set_vertex_input_binding_descriptions(const std::vector<VkVertexInputBindingDescription> &descriptions);

    /// Add a vertex input binding description
    /// @tparam T The vertex structure
    /// @param binding The vertex input binding descriptions
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    template <typename T>
    [[nodiscard]] GraphicsStage *set_vertex_input_binding_descriptions(std::uint32_t binding = 0) {
        m_vertex_input_binding_descriptions.push_back({
            .binding = binding,
            .stride = sizeof(T),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        });
        return this;
    }

    /// Set the viewport in VkPipelineViewportStateCreateInfo
    /// There is another method called set_viewports in case multiple viewports will be used
    /// @param viewport The viewport in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_viewport(const VkViewport &viewport);

    /// Set the viewport in VkPipelineViewportStateCreateInfo (convert VkExtent2D to VkViewport)
    /// @param extent The extent of the viewport
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_viewport(const VkExtent2D &extent);

    /// Set the viewport in VkPipelineViewportStateCreateInfo
    /// @param viewports The viewports in VkPipelineViewportStateCreateInfo
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_viewports(const std::vector<VkViewport> &viewports);

    /// Set the wireframe mode
    /// @param wireframe ``true`` if wireframe is enabled
    /// @return A reference to the dereferenced this pointer (allows method calls to be chained)
    [[nodiscard]] GraphicsStage *set_wireframe(VkBool32 wireframe);
};

// TODO: Add wrapper::Allocation that can be made by doing `device->make<Allocation>(...)`.
class PhysicalResource : public RenderGraphObject {
    friend RenderGraph;

protected:
    const wrapper::Device &m_device;

    explicit PhysicalResource(const wrapper::Device &device) : m_device(device) {}

public:
    PhysicalResource(const PhysicalResource &) = delete;
    PhysicalResource(PhysicalResource &&) = delete;
    ~PhysicalResource() override = default;

    PhysicalResource &operator=(const PhysicalResource &) = delete;
    PhysicalResource &operator=(PhysicalResource &&) = delete;
};

class PhysicalBuffer : public PhysicalResource {
    friend RenderGraph;

private:
    std::unique_ptr<wrapper::Buffer> m_buffer;
    VkDescriptorBufferInfo m_descriptor_buffer_info{};

public:
    explicit PhysicalBuffer(const wrapper::Device &device) : PhysicalResource(device) {}
    PhysicalBuffer(const PhysicalBuffer &) = delete;
    PhysicalBuffer(PhysicalBuffer &&) = delete;
    ~PhysicalBuffer() override = default;

    PhysicalBuffer &operator=(const PhysicalBuffer &) = delete;
    PhysicalBuffer &operator=(PhysicalBuffer &&) = delete;
};

class PhysicalImage : public PhysicalResource {
    friend RenderGraph;

private:
    std::unique_ptr<wrapper::Image> m_img;

public:
    explicit PhysicalImage(const wrapper::Device &device) : PhysicalResource(device) {}
    PhysicalImage(const PhysicalImage &) = delete;
    PhysicalImage(PhysicalImage &&) = delete;
    ~PhysicalImage() override = default;

    PhysicalImage &operator=(const PhysicalImage &) = delete;
    PhysicalImage &operator=(PhysicalImage &&) = delete;

    [[nodiscard]] VkImageView image_view() const noexcept {
        return m_img->image_view();
    }
};

class PhysicalBackBuffer : public PhysicalResource {
    friend RenderGraph;

private:
    const wrapper::Swapchain &m_swapchain;

public:
    PhysicalBackBuffer(const wrapper::Device &device, const wrapper::Swapchain &swapchain)
        : PhysicalResource(device), m_swapchain(swapchain) {}
    PhysicalBackBuffer(const PhysicalBackBuffer &) = delete;
    PhysicalBackBuffer(PhysicalBackBuffer &&) = delete;
    ~PhysicalBackBuffer() override = default;

    PhysicalBackBuffer &operator=(const PhysicalBackBuffer &) = delete;
    PhysicalBackBuffer &operator=(PhysicalBackBuffer &&) = delete;
};

class PhysicalStage : public PhysicalResource {
    friend RenderGraph;

private:
    VkDescriptorSet m_descriptor_set;
    VkDescriptorSetLayout m_descriptor_set_layout;

    std::unique_ptr<wrapper::pipelines::GraphicsPipeline> m_pipeline;
    std::unique_ptr<wrapper::pipelines::PipelineLayout> m_pipeline_layout;

public:
    explicit PhysicalStage(const wrapper::Device &device) : PhysicalResource(device) {}
    PhysicalStage(const PhysicalStage &) = delete;
    PhysicalStage(PhysicalStage &&) = delete;
    ~PhysicalStage() override = default;

    PhysicalStage &operator=(const PhysicalStage &) = delete;
    PhysicalStage &operator=(PhysicalStage &&) = delete;

    /// @brief Retrieve the pipeline layout of this physical stage.
    // TODO: This can be removed once descriptors are properly implemented in the render graph.
    [[nodiscard]] VkPipelineLayout pipeline_layout() const {
        return m_pipeline_layout->pipeline_layout();
    }
};

class PhysicalGraphicsStage : public PhysicalStage {
    friend RenderGraph;

private:
    std::unique_ptr<wrapper::RenderPass> m_render_pass;
    std::vector<wrapper::Framebuffer> m_framebuffers;

public:
    explicit PhysicalGraphicsStage(const wrapper::Device &device) : PhysicalStage(device) {}
    PhysicalGraphicsStage(const PhysicalGraphicsStage &) = delete;
    PhysicalGraphicsStage(PhysicalGraphicsStage &&) = delete;
    ~PhysicalGraphicsStage() override = default;

    PhysicalGraphicsStage &operator=(const PhysicalGraphicsStage &) = delete;
    PhysicalGraphicsStage &operator=(PhysicalGraphicsStage &&) = delete;
};

class RenderGraph {
private:
    wrapper::Device &m_device;
    const wrapper::Swapchain &m_swapchain;
    std::shared_ptr<spdlog::logger> m_log{spdlog::default_logger()->clone("render-graph")};

    // Vectors of render resources and stages.
    std::vector<std::unique_ptr<BufferResource>> m_buffer_resources;
    std::vector<std::unique_ptr<TextureResource>> m_texture_resources;
    std::vector<std::unique_ptr<RenderStage>> m_stages;

    // Descriptor management
    wrapper::descriptors::DescriptorSetAllocator m_descriptor_set_allocator;
    wrapper::descriptors::DescriptorSetLayoutCache m_descriptor_set_layout_cache;
    wrapper::descriptors::DescriptorSetLayoutBuilder m_descriptor_set_layout_builder;
    wrapper::descriptors::DescriptorSetUpdater m_descriptor_set_updater;
    // We only store the indices of the uniform buffer resources which have been updated
    std::vector<std::size_t> m_indices_of_updated_uniform_buffers;
    // Each uniform buffer resource can be read from multiple render stages
    std::vector<std::vector<RenderStage *>> m_uniform_buffer_reading_stages;

    // Stage execution order.
    std::vector<RenderStage *> m_stage_stack;

    void create_buffer(PhysicalBuffer &physical, BufferResource *buffer_resource);

    void create_framebuffers(PhysicalGraphicsStage &physical, const GraphicsStage *stage);
    void determine_stage_order(const RenderResource *target);
    /// Create physical resources
    /// For now, each buffer or texture resource maps directly to either a VkBuffer or VkImage respectively
    /// Every physical resource also has a VmaAllocation.
    /// TODO: Resource aliasing (i.e. reusing the same physical resource for multiple resources)
    void create_buffer_resources();
    void create_texture_resources();
    void build_descriptor_sets(const RenderStage *stage);
    void create_push_constant_ranges(GraphicsStage *stage);
    void create_pipeline_layout(PhysicalGraphicsStage &physical, GraphicsStage *stage);
    void create_graphics_pipeline(PhysicalGraphicsStage &physical, GraphicsStage *stage);

    /// We associate each uniform buffer to the stages which read from it so we know which descriptors in which stages
    /// need to be updated if the uniform buffer has been updated/recreated
    void collect_render_stages_reading_from_uniform_buffers();

    /// Update the dynamic uniform buffers
    /// @note Keep in mind this function will be called once every frame
    void update_dynamic_buffers();

    /// Update the push constant range
    void update_push_constant_ranges();

    /// Update the descriptor sets for uniform buffers
    /// @note Keep in mind this function will be called once every frame
    void update_uniform_buffer_descriptor_sets();

    /// Update the descriptor sets for textures
    void update_texture_descriptor_sets();

    // Functions for building stage related vulkan objects.
    void record_command_buffer(const RenderStage *, const wrapper::CommandBuffer &cmd_buf, std::uint32_t image_index);

    // Functions for building graphics stage related vulkan objects.
    void build_render_pass(const GraphicsStage *, PhysicalGraphicsStage &);

public:
    RenderGraph(wrapper::Device &device, const wrapper::Swapchain &swapchain)
        : m_device(device), m_swapchain(swapchain), m_descriptor_set_allocator(m_device),
          m_descriptor_set_layout_cache(device), m_descriptor_set_layout_builder(device, m_descriptor_set_layout_cache),
          m_descriptor_set_updater(device) {}

    /// @brief Adds either a render resource or render stage to the render graph.
    /// @return A mutable reference to the just-added resource or stage
    template <typename T, typename... Args>
    T *add(Args &&...args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        if constexpr (std::is_same_v<T, BufferResource>) {
            return static_cast<T *>(m_buffer_resources.emplace_back(std::move(ptr)).get());
        } else if constexpr (std::is_same_v<T, TextureResource>) {
            return static_cast<T *>(m_texture_resources.emplace_back(std::move(ptr)).get());
        } else if constexpr (std::is_base_of_v<RenderStage, T>) {
            return static_cast<T *>(m_stages.emplace_back(std::move(ptr)).get());
        } else {
            static_assert(!std::is_same_v<T, T>, "T must be a RenderResource or RenderStage");
        }
    }

    /// @brief Compiles the render graph resources/stages into physical vulkan objects.
    /// @param target The target resource of the render graph (usually the back buffer)
    void compile(const RenderResource *target);

    /// @brief Submits the command frame's command buffers for drawing.
    /// @param image_index The current image index, retrieved from Swapchain::acquire_next_image
    /// @param cmd_buf The command buffer
    void render(std::uint32_t image_index, const wrapper::CommandBuffer &cmd_buf);
};

} // namespace inexor::vulkan_renderer
