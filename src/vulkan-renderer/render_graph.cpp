#include "inexor/vulkan-renderer/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/vk_tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>
#include <volk.h>

#include <array>
#include <cassert>
#include <functional>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer {

RenderStage *RenderStage::writes_to(const RenderResource *resource) {
    m_writes.push_back(resource);
    return this;
}

RenderStage *RenderStage::reads_from(RenderResource *resource, const VkShaderStageFlags shader_stage) {
    m_reads.push_back(std::make_pair(resource, shader_stage));
    return this;
}

RenderStage *RenderStage::reads_from(RenderResource *resource) {
    auto *buffer_resource = resource->as<BufferResource>();
    if (buffer_resource != nullptr) {
        // Omitting the shader stage is only allowed for vertex buffers and index buffers!
        m_reads.push_back(std::make_pair(resource, std::nullopt));
    } else {
        throw std::invalid_argument("Error: Omitting the shader stage when specifying reads_from is only alowed for "
                                    "vertex buffers and index buffers!");
    }
    return this;
}

GraphicsStage *GraphicsStage::add_shader(const VkPipelineShaderStageCreateInfo &shader_stage) {
    m_shader_stages.push_back(shader_stage);
    return this;
}

GraphicsStage *GraphicsStage::add_shader(const wrapper::Shader &shader) {
    return add_shader(wrapper::make_info<VkPipelineShaderStageCreateInfo>({
        .stage = shader.type(),
        .module = shader.module(),
        .pName = shader.entry_point().c_str(),
    }));
}

GraphicsStage *GraphicsStage::add_color_blend_attachment(const VkPipelineColorBlendAttachmentState &attachment) {
    m_color_blend_attachment = attachment;
    return this;
}

GraphicsStage *GraphicsStage::add_vertex_input_attribute(const VkVertexInputAttributeDescription &description) {
    m_vertex_input_attribute_descriptions.push_back(description);
    return this;
}

GraphicsStage *GraphicsStage::add_vertex_input_binding(const VkVertexInputBindingDescription &description) {
    m_vertex_input_binding_descriptions.push_back(description);
    return this;
}

VkGraphicsPipelineCreateInfo GraphicsStage::make_create_info() {
    m_vertex_input_sci = wrapper::make_info<VkPipelineVertexInputStateCreateInfo>({
        .vertexBindingDescriptionCount = static_cast<std::uint32_t>(m_vertex_input_binding_descriptions.size()),
        .pVertexBindingDescriptions = m_vertex_input_binding_descriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(m_vertex_input_attribute_descriptions.size()),
        .pVertexAttributeDescriptions = m_vertex_input_attribute_descriptions.data(),

    });

    m_viewport_sci = wrapper::make_info<VkPipelineViewportStateCreateInfo>({
        .viewportCount = static_cast<uint32_t>(m_viewports.size()),
        .pViewports = m_viewports.data(),
        .scissorCount = static_cast<uint32_t>(m_scissors.size()),
        .pScissors = m_scissors.data(),
    });

    if (!m_dynamic_states.empty()) {
        m_dynamic_states_sci = wrapper::make_info<VkPipelineDynamicStateCreateInfo>({
            .dynamicStateCount = static_cast<std::uint32_t>(m_dynamic_states.size()),
            .pDynamicStates = m_dynamic_states.data(),
        });
    }

    return wrapper::make_info<VkGraphicsPipelineCreateInfo>({
        .stageCount = static_cast<std::uint32_t>(m_shader_stages.size()),
        .pStages = m_shader_stages.data(),
        .pVertexInputState = &m_vertex_input_sci,
        .pInputAssemblyState = &m_input_assembly_sci,
        .pTessellationState = &m_tesselation_sci,
        .pViewportState = &m_viewport_sci,
        .pRasterizationState = &m_rasterization_sci,
        .pMultisampleState = &m_multisample_sci,
        .pDepthStencilState = &m_depth_stencil_sci,
        .pColorBlendState = &m_color_blend_sci,
        .pDynamicState = &m_dynamic_states_sci,
        .layout = m_pipeline_layout,
        .renderPass = m_render_pass,
    });
}

GraphicsStage *GraphicsStage::set_color_blend(const VkPipelineColorBlendStateCreateInfo &color_blend) {
    m_color_blend_sci = color_blend;
    return this;
}

GraphicsStage *GraphicsStage::set_culling_mode(const VkBool32 culling_enabled) {
    spdlog::warn("Culling is disabled, which could have negative effects on the performance!");
    m_rasterization_sci.cullMode = culling_enabled ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    return this;
}

GraphicsStage *GraphicsStage::set_depth_stencil(const VkPipelineDepthStencilStateCreateInfo &depth_stencil) {
    m_depth_stencil_sci = depth_stencil;
    return this;
}

GraphicsStage *GraphicsStage::set_dynamic_states(const std::vector<VkDynamicState> &dynamic_states) {
    assert(!dynamic_states.empty());
    m_dynamic_states = dynamic_states;
    return this;
}

GraphicsStage *GraphicsStage::set_line_width(const float width) {
    m_rasterization_sci.lineWidth = width;
    return this;
}

GraphicsStage *GraphicsStage::set_multisampling(const VkSampleCountFlagBits sample_count,
                                                const float min_sample_shading) {
    m_multisample_sci.rasterizationSamples = sample_count;
    m_multisample_sci.minSampleShading = min_sample_shading;
    return this;
}

GraphicsStage *GraphicsStage::set_pipeline_layout(const VkPipelineLayout layout) {
    assert(layout);
    m_pipeline_layout = layout;
    return this;
}

GraphicsStage *GraphicsStage::set_primitive_topology(const VkPrimitiveTopology topology) {
    m_input_assembly_sci.topology = topology;
    return this;
}

GraphicsStage *GraphicsStage::set_rasterization(const VkPipelineRasterizationStateCreateInfo &rasterization) {
    m_rasterization_sci = rasterization;
    return this;
}

GraphicsStage *GraphicsStage::set_render_pass(const VkRenderPass render_pass) {
    assert(render_pass);
    m_render_pass = render_pass;
    return this;
}

GraphicsStage *GraphicsStage::set_scissor(const VkRect2D &scissor) {
    m_scissors = {scissor};
    m_viewport_sci.scissorCount = 1;
    m_viewport_sci.pScissors = m_scissors.data();
    return this;
}

GraphicsStage *GraphicsStage::set_scissor(const VkExtent2D &extent) {
    return set_scissor({
        // Convert VkExtent2D to VkRect2D
        .extent = extent,
    });
}

GraphicsStage *GraphicsStage::set_scissors(const std::vector<VkRect2D> &scissors) {
    assert(!scissors.empty());
    m_scissors = scissors;
    m_viewport_sci.scissorCount = static_cast<std::uint32_t>(scissors.size());
    m_viewport_sci.pScissors = scissors.data();
    return this;
}

GraphicsStage *GraphicsStage::set_shaders(const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages) {
    assert(!shader_stages.empty());
    m_shader_stages = shader_stages;
    return this;
}

GraphicsStage *GraphicsStage::set_tesselation_control_point_count(const std::uint32_t control_point_count) {
    m_tesselation_sci.patchControlPoints = control_point_count;
    return this;
}

GraphicsStage *GraphicsStage::set_vertex_input_attribute_descriptions(
    const std::vector<VkVertexInputAttributeDescription> &descriptions) {
    assert(!descriptions.empty());
    m_vertex_input_attribute_descriptions = descriptions;
    return this;
}

GraphicsStage *
GraphicsStage::set_vertex_input_binding_descriptions(const std::vector<VkVertexInputBindingDescription> &descriptions) {
    assert(!descriptions.empty());
    m_vertex_input_binding_descriptions = descriptions;
    return this;
}

GraphicsStage *GraphicsStage::set_viewport(const VkViewport &viewport) {
    m_viewports = {viewport};
    m_viewport_sci.viewportCount = 1;
    m_viewport_sci.pViewports = m_viewports.data();
    return this;
}

GraphicsStage *GraphicsStage::set_viewport(const VkExtent2D &extent) {
    return set_viewport({
        // Convert VkExtent2D to VkViewport
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .maxDepth = 1.0f,
    });
}

GraphicsStage *GraphicsStage::set_viewports(const std::vector<VkViewport> &viewports) {
    assert(!viewports.empty());
    m_viewports = viewports;
    m_viewport_sci.viewportCount = static_cast<std::uint32_t>(m_viewports.size());
    m_viewport_sci.pViewports = m_viewports.data();
    return this;
}

GraphicsStage *GraphicsStage::set_wireframe(const VkBool32 wireframe) {
    m_rasterization_sci.polygonMode = (wireframe == VK_TRUE) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    return this;
}

void RenderGraph::record_command_buffer(const RenderStage *stage, const wrapper::CommandBuffer &cmd_buf,
                                        const std::uint32_t image_index) {
    const PhysicalStage &physical = *stage->m_physical;

    // Record render pass for graphics stages.
    const auto *graphics_stage = stage->as<GraphicsStage>();

    // TODO: graphics_stage == nullptr should throw an exception?
    if (graphics_stage != nullptr) {
        const auto *phys_graphics_stage = physical.as<PhysicalGraphicsStage>();
        assert(phys_graphics_stage != nullptr);

        std::array<VkClearValue, 2> clear_values{};
        if (graphics_stage->m_clears_screen) {
            clear_values[0].color = graphics_stage->m_clear_value.color;
            clear_values[1].depthStencil = {1.0f, 0};
        }

        cmd_buf.begin_render_pass(wrapper::make_info<VkRenderPassBeginInfo>({
            .renderPass = phys_graphics_stage->m_render_pass->render_pass(),
            .framebuffer = phys_graphics_stage->m_framebuffers.at(image_index).get(),
            .renderArea{
                .extent = m_swapchain.extent(),
            },
            .clearValueCount = static_cast<std::uint32_t>(clear_values.size()),
            .pClearValues = clear_values.data(),
        }));
    }

    std::vector<VkBuffer> vertex_buffers;
    for (const auto &resource : stage->m_reads) {
        const auto *buffer_resource = resource.first->as<BufferResource>();
        if (buffer_resource == nullptr) {
            continue;
        }
        auto *physical_buffer = buffer_resource->m_physical->as<PhysicalBuffer>();
        if (physical_buffer->m_buffer == nullptr) {
            continue;
        }
        if (buffer_resource->m_usage == BufferUsage::INDEX_BUFFER) {
            // Note that in Vulkan you can bind multiple vertex buffers, but only one index buffer
            cmd_buf.bind_index_buffer(physical_buffer->m_buffer->buffer());
        } else if (buffer_resource->m_usage == BufferUsage::VERTEX_BUFFER) {
            vertex_buffers.push_back(physical_buffer->m_buffer->buffer());
        }
    }

    if (!vertex_buffers.empty()) {
        // Note that in Vulkan you can bind multiple vertex buffers, but only one index buffer
        cmd_buf.bind_vertex_buffers(vertex_buffers);
    }

    cmd_buf.bind_pipeline(physical.m_pipeline->pipeline());

    for (auto &push_constant : stage->m_push_constants) {
        cmd_buf.push_constants(physical.m_pipeline_layout->pipeline_layout(), push_constant.m_push_constant.stageFlags,
                               push_constant.m_push_constant.size, push_constant.m_push_constant_data);
    }

    cmd_buf.bind_descriptor_sets(physical.m_descriptor_sets, physical.m_pipeline_layout->pipeline_layout());

    stage->m_on_record(cmd_buf);

    if (graphics_stage != nullptr) {
        cmd_buf.end_render_pass();
    }

    // TODO: Find a more performant solution instead of placing a full memory barrier after each stage!
    cmd_buf.full_barrier();
}

void RenderGraph::build_render_pass(const GraphicsStage *stage, PhysicalGraphicsStage &physical) {
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colour_refs;
    std::vector<VkAttachmentReference> depth_refs;

    // Build vulkan attachments: For every texture resource that stage writes to, we create a corresponding
    // VkAttachmentDescription and attach it to the render pass.
    // TODO(GH-203): Support multisampled attachments
    for (std::size_t i = 0; i < stage->m_writes.size(); i++) {
        const auto *resource = stage->m_writes[i];
        const auto *texture = resource->as<TextureResource>();
        if (texture == nullptr) {
            continue;
        }

        VkAttachmentDescription attachment{
            .format = texture->m_format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = stage->m_clears_screen ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        switch (texture->m_usage) {
        case TextureUsage::BACK_BUFFER:
            if (!stage->m_clears_screen) {
                attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            }
            attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            colour_refs.push_back({static_cast<std::uint32_t>(i), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
            break;
        case TextureUsage::DEPTH_STENCIL_BUFFER:
            attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depth_refs.push_back({static_cast<std::uint32_t>(i), attachment.finalLayout});
            break;
        default:
            attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colour_refs.push_back({static_cast<std::uint32_t>(i), attachment.finalLayout});
            break;
        }
        attachments.push_back(attachment);
    }

    const std::vector<VkSubpassDescription> subpasses{
        {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = static_cast<std::uint32_t>(colour_refs.size()),
            .pColorAttachments = colour_refs.data(),
            .pDepthStencilAttachment = !depth_refs.empty() ? depth_refs.data() : nullptr,
        },
    };

    // Build a simple subpass that just waits for the output colour vector to be written by the fragment shader
    const std::vector<VkSubpassDependency> dependencies{
        {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        },
    };

    physical.m_render_pass =
        std::make_unique<wrapper::RenderPass>(m_device, attachments, subpasses, dependencies, stage->name());
}

void RenderGraph::create_buffer_resources() {
    m_log->trace("Allocating {} physical buffer{}:", m_buffer_resources.size(),
                 m_buffer_resources.size() > 1 ? "s" : "");

    for (auto &buffer_resource : m_buffer_resources) {
        // TODO: Move this to representation header
        const std::unordered_map<BufferUsage, std::string> buffer_usage_name{
            {BufferUsage::VERTEX_BUFFER, "VERTEX_BUFFER"},
            {BufferUsage::INDEX_BUFFER, "INDEX_BUFFER"},
            {BufferUsage::UNIFORM_BUFFER, "UNIFORM_BUFFER"},
        };

        // Call the buffer's update function
        buffer_resource->m_on_update();

        m_log->trace("   - {}\t [type: {},\t size: {} bytes]", buffer_resource->m_name,
                     buffer_usage_name.at(buffer_resource->m_usage), buffer_resource->m_data_size);
        buffer_resource->m_physical = std::make_shared<PhysicalBuffer>(m_device);
    }
}

void RenderGraph::create_texture_resources() {
    m_log->trace("Allocating {} physical texture{}:", m_texture_resources.size(),
                 m_texture_resources.size() > 1 ? "s" : "");

    for (auto &texture_resource : m_texture_resources) {
        // TODO: Move this to representation header
        const std::unordered_map<TextureUsage, std::string> texture_usage_name{
            {TextureUsage::BACK_BUFFER, "BACK_BUFFER"},
            {TextureUsage::DEPTH_STENCIL_BUFFER, "DEPTH_STENCIL_BUFFER"},
            {TextureUsage::NORMAL, "NORMAL"},
        };

        m_log->trace("   - {}\t [format: {}, usage: {}]", texture_resource->m_name,
                     vk_tools::as_string(texture_resource->m_format), texture_usage_name.at(texture_resource->m_usage));

        // Back buffer gets special handling.
        if (texture_resource->m_usage == TextureUsage::BACK_BUFFER) {
            // TODO: Move image views from wrapper::Swapchain to PhysicalBackBuffer.
            texture_resource->m_physical = std::make_shared<PhysicalBackBuffer>(m_device, m_swapchain);
        } else {
            auto physical = std::make_shared<PhysicalImage>(m_device);
            texture_resource->m_physical = physical;

            physical->m_img = std::make_unique<wrapper::Image>(
                m_device, texture_resource->m_format, m_swapchain.extent().width, m_swapchain.extent().height,
                texture_resource->m_usage == TextureUsage::DEPTH_STENCIL_BUFFER
                    ? static_cast<VkImageUsageFlags>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                    : static_cast<VkImageUsageFlags>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
                static_cast<VkImageAspectFlags>(texture_resource->m_usage == TextureUsage::DEPTH_STENCIL_BUFFER
                                                    ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
                                                    : VK_IMAGE_ASPECT_COLOR_BIT),
                // TODO: Apply internal debug name to the images
                "Rendergraph image");
        }
    }
}

void RenderGraph::build_descriptor_sets(const RenderStage *stage) {
    // Use the descriptor builder to assemble the descriptor
    for (auto &read_resource : stage->m_reads) {
        // For simplicity reasons, check if it's an external texture resource first
        auto *external_texture = read_resource.first->as<ExternalTextureResource>();
        if (external_texture != nullptr) {
            external_texture->m_descriptor_image_info = VkDescriptorImageInfo{
                .sampler = external_texture->m_texture.sampler(),
                .imageView = external_texture->m_texture.image_view(),
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };
            // Add combined image sampler to builder
            descriptor_builder.bind_image(&external_texture->m_descriptor_image_info, read_resource.second.value());
        }
        auto *physical = read_resource.first->m_physical->as<PhysicalBuffer>();
        if (physical != nullptr) {
            // This is a buffer, so check if it's a uniform buffer
            auto *buffer = read_resource.first->as<BufferResource>();
            if (buffer != nullptr) {
                if (buffer->m_usage == BufferUsage::UNIFORM_BUFFER) {
                    // Build the buffer's descriptor buffer info
                    physical->m_descriptor_buffer_info = VkDescriptorBufferInfo{
                        .buffer = physical->m_buffer->buffer(),
                        .offset = 0,
                        .range = buffer->m_data_size,
                    };
                    // Add uniform buffer to builder
                    descriptor_builder.bind_uniform_buffer(&physical->m_descriptor_buffer_info,
                                                           read_resource.second.value());
                }
            }
        }
    }

    // Don't forget to clear the previous descriptor sets and descriptor set layouts
    stage->m_physical->m_descriptor_sets.clear();
    stage->m_physical->m_descriptor_set_layouts.clear();

    // Build the descriptor and store descriptor set and descriptor set layout
    const auto descriptor = descriptor_builder.build();
    stage->m_physical->m_descriptor_sets.push_back(descriptor.first);
    stage->m_physical->m_descriptor_set_layouts.push_back(descriptor.second);
}

void RenderGraph::create_push_constant_ranges(GraphicsStage *stage) {
    // Collect the push constant ranges of this stage into one std::vector
    stage->m_push_constant_ranges.reserve(stage->m_push_constants.size());
    for (const auto &push_constant : stage->m_push_constants) {
        stage->m_push_constant_ranges.push_back(push_constant.m_push_constant);
    }
}

void RenderGraph::create_pipeline_layout(PhysicalGraphicsStage &physical, GraphicsStage *stage) {
    physical.m_pipeline_layout =
        std::make_unique<wrapper::pipelines::PipelineLayout>(m_device, stage->m_physical->m_descriptor_set_layouts,
                                                             stage->m_push_constant_ranges, "graphics pipeline layout");
}

void RenderGraph::create_graphics_pipeline(PhysicalGraphicsStage &physical, GraphicsStage *stage) {
    physical.m_pipeline = std::make_unique<wrapper::pipelines::GraphicsPipeline>(
        m_device,
        stage
            ->set_color_blend(wrapper::make_info<VkPipelineColorBlendStateCreateInfo>({
                .attachmentCount = 1,
                .pAttachments = &stage->m_color_blend_attachment,
            }))
            ->set_depth_stencil(wrapper::make_info<VkPipelineDepthStencilStateCreateInfo>({
                .depthTestEnable = stage->m_depth_test ? VK_TRUE : VK_FALSE,
                .depthWriteEnable = stage->m_depth_write ? VK_TRUE : VK_FALSE,
                .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
            }))
            ->set_pipeline_layout(physical.m_pipeline_layout->pipeline_layout())
            ->set_render_pass(physical.m_render_pass->render_pass())
            ->set_scissor(m_swapchain.extent())
            ->set_viewport(m_swapchain.extent())
            ->make_create_info(),
        "graphics pipeline");
}

void RenderGraph::determine_stage_order(const RenderResource *target) {
    // Build a simple helper map to lookup a resource's writers.
    std::unordered_map<const RenderResource *, std::vector<RenderStage *>> writers;
    for (auto &stage : m_stages) {
        for (const auto *resource : stage->m_writes) {
            writers[resource].push_back(stage.get());
        }
    }

    // TODO: Implement check_for_cycles_in_graph();

    // Post order depth first search. Note that this doesn't do any colouring, so it only works on acyclic graphs.
    // TODO(GH-204): Stage graph validation (ensuring no cycles, etc.).
    // TODO: Move away from recursive dfs algo.
    std::function<void(RenderStage *)> dfs = [&](RenderStage *stage) {
        for (const auto &resource : stage->m_reads) {
            for (auto *writer : writers[resource.first]) {
                dfs(writer);
            }
        }
        m_stage_stack.push_back(stage);
    };

    // DFS starting from writers of target (initial stage executants).
    for (auto *stage : writers[target]) {
        dfs(stage);
    }

    m_log->trace("Final order of {} stages:", m_stage_stack.size());
    for (auto *stage : m_stage_stack) {
        m_log->trace("   - {}\t [reads: {}, writes: {}, push constant ranges: {}]", stage->m_name,
                     stage->m_reads.size(), stage->m_writes.size(), stage->m_push_constants.size());
    }
}

void RenderGraph::create_framebuffers(PhysicalGraphicsStage &physical, const GraphicsStage *stage) {
    // If we write to at least one texture, we need to make framebuffers.
    if (!stage->m_writes.empty()) {
        // For every texture that this stage writes to, we need to attach it to the framebuffer.
        std::vector<const PhysicalBackBuffer *> back_buffers;
        std::vector<const PhysicalImage *> images;
        for (const auto *resource : stage->m_writes) {
            if (const auto *texture = resource->as<TextureResource>()) {
                const auto &physical_texture = *texture->m_physical;
                if (const auto *back_buffer = physical_texture.as<PhysicalBackBuffer>()) {
                    back_buffers.push_back(back_buffer);
                } else if (const auto *image = physical_texture.as<PhysicalImage>()) {
                    images.push_back(image);
                }
            }
        }

        std::vector<VkImageView> image_views;
        image_views.reserve(back_buffers.size() + images.size());
        for (auto *const img_view : m_swapchain.image_views()) {
            std::fill_n(std::back_inserter(image_views), back_buffers.size(), img_view);
            for (const auto *image : images) {
                image_views.push_back(image->image_view());
            }
            physical.m_framebuffers.emplace_back(m_device, physical.m_render_pass->render_pass(), image_views,
                                                 m_swapchain, "Framebuffer");
            image_views.clear();
        }
    }
}

void RenderGraph::compile(const RenderResource *target) {
    // TODO(GH-204): Better logging and input validation.
    // TODO: Many opportunities for optimisation.
    determine_stage_order(target);
    create_buffer_resources();
    update_dynamic_buffers();
    create_texture_resources();

    // Create physical stages. Each render stage maps to a vulkan pipeline (either compute or graphics) and a list of
    // command buffers. Each graphics stage also maps to a vulkan render pass.
    for (auto *stage : m_stage_stack) {
        if (auto *graphics_stage = stage->as<GraphicsStage>()) {
            // TODO: Can't we simplify this?
            auto physical_ptr = std::make_unique<PhysicalGraphicsStage>(m_device);
            auto &physical = *physical_ptr;
            graphics_stage->m_physical = std::move(physical_ptr);

            build_render_pass(graphics_stage, physical);
            build_descriptor_sets(graphics_stage);
            create_push_constant_ranges(graphics_stage);
            create_pipeline_layout(physical, graphics_stage);
            create_graphics_pipeline(physical, graphics_stage);
            create_framebuffers(physical, graphics_stage);
        }
    }
}

void RenderGraph::update_push_constant_ranges(const RenderStage *stage) {
    for (auto &push_constant : stage->m_push_constants) {
        push_constant.m_on_update();
    }
}

void RenderGraph::create_buffer(PhysicalBuffer &physical, const BufferResource *buffer_resource) {
    // This translates the rendergraph's internal buffer usage to Vulkam buffer usage flags
    const std::unordered_map<BufferUsage, VkBufferUsageFlags> buffer_usage{
        {BufferUsage::VERTEX_BUFFER, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
        {BufferUsage::INDEX_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
        {BufferUsage::UNIFORM_BUFFER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
    };

    // TODO: Implement a buffer.recreate(); method (No need to destroy the unique pointer!)
    physical.m_buffer = std::make_unique<wrapper::Buffer>(
        m_device, buffer_resource->m_data_size, buffer_resource->m_data,
        // TODO: This does not support staging buffers yet because of VMA_MEMORY_USAGE_CPU_TO_GPU!
        buffer_usage.at(buffer_resource->m_usage), VMA_MEMORY_USAGE_CPU_TO_GPU, buffer_resource->name());
}

void RenderGraph::update_dynamic_buffers() {
    // If a uniform buffer is recreated, we need to update the descriptor sets for that stage
    bool update_stage_descriptor_sets = false;

    for (auto &buffer_resource : m_buffer_resources) {
        auto &physical = *buffer_resource->m_physical->as<PhysicalBuffer>();

        // Call the buffer's update function
        buffer_resource->m_on_update();

        if (buffer_resource->m_data_upload_needed) {
            // Check if this buffer has already been created
            if (physical.m_buffer != nullptr) {
                physical.m_buffer.reset();
                physical.m_buffer = nullptr;

                // If it's a uniform buffer, we need to update descriptors!
                if (buffer_resource->m_usage == BufferUsage::UNIFORM_BUFFER) {
                    update_stage_descriptor_sets = true;
                }
            }
            create_buffer(physical, buffer_resource.get());

            // TODO: Implement updates which requires staging buffers!
            std::memcpy(physical.m_buffer->memory(), buffer_resource->m_data, buffer_resource->m_data_size);

            // Check if any descriptor set update is necessary
            if (update_stage_descriptor_sets) {
                for (auto &stage : m_stage_stack) {
                    auto *graphics_stage = stage->as<GraphicsStage>();
                    if (graphics_stage == nullptr) {
                        continue;
                    }
                    for (auto &[key, value] : graphics_stage->m_reads) {
                        auto *stage_physical = key->m_physical->as<PhysicalBuffer>();
                        // Check if this stage is reading from this buffer
                        if (key == buffer_resource.get()) {
                            stage_physical->m_descriptor_buffer_info.buffer = physical.m_buffer->buffer();
                            build_descriptor_sets(graphics_stage);
                        }
                    }
                }
                // Descriptor update is done
                update_stage_descriptor_sets = false;
            }
        }
    }
}

void RenderGraph::render(const std::uint32_t image_index, const wrapper::CommandBuffer &cmd_buf) {
    // TODO: This is a waste of performance
    for (const auto &stage : m_stage_stack) {
        stage->m_on_update();
        update_push_constant_ranges(stage);
    }
    // TODO: This is a waste of performance
    update_dynamic_buffers();
    for (const auto &stage : m_stage_stack) {
        record_command_buffer(stage, cmd_buf, image_index);
    }
}

} // namespace inexor::vulkan_renderer
