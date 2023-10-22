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
    // Omitting the shader stage is only allowed for vertex buffers and index buffers!
    if (buffer_resource != nullptr) {
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

VkGraphicsPipelineCreateInfo GraphicsStage::make_create_info(const VkFormat swapchain_img_format) {
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

    m_swapchain_img_format = swapchain_img_format;

    m_pipeline_rendering_ci = wrapper::make_info<VkPipelineRenderingCreateInfo>({
        // Because we use pipeline_rendering_ci as pNext parameter in VkGraphicsPipelineCreateInfo,
        // we must end the pNext chain here by setting it to nullptr explicitely!
        .pNext = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &m_swapchain_img_format,
    });

    return wrapper::make_info<VkGraphicsPipelineCreateInfo>({
        .pNext = &m_pipeline_rendering_ci,
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
        .renderPass = VK_NULL_HANDLE, // We use dynamic rendering
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

void RenderGraph::record_command_buffer(const bool first_stage, const bool last_stage, const RenderStage *stage,
                                        const wrapper::CommandBuffer &cmd_buf, const std::uint32_t image_index) {
    const PhysicalStage &physical = *stage->m_physical;
    const auto *graphics_stage = stage->as<GraphicsStage>();

    float color[4];
    color[0] = 1.0f;
    color[1] = 0.0f;
    color[2] = 0.0f;
    color[3] = 1.0f;

    cmd_buf.begin_debug_label_region(stage->name(), color);

    // TODO: Is there a way to further abstract image layout transitions depending on type and usage?
    // Wouldn't we simply have to iterate through all texture reads of the current stage and process them?
    // Also, can't we just process all reads as attachments here because of dynamic rendering?

    if (first_stage) {
        float color[4];
        color[0] = 0.0f;
        color[1] = 0.0f;
        color[2] = 1.0f;
        color[3] = 0.4f;
        cmd_buf.insert_debug_label("Hello world", color);
        cmd_buf.change_image_layout(m_swapchain.image(image_index), VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    const auto *phys_graphics_stage = physical.as<PhysicalStage>();
    assert(phys_graphics_stage != nullptr);

    const auto color_attachment = wrapper::make_info<VkRenderingAttachmentInfo>({
        .imageView = m_swapchain.image_view(image_index),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = graphics_stage->m_clears_screen ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue =
            {
                .color = graphics_stage->m_clear_value.color,
            },
    });

    VkImageView depth_buffer_img_view{VK_NULL_HANDLE};
    VkImage depth_buffer{VK_NULL_HANDLE};

    // Loop through all writes and find the depth buffer
    for (const auto &resource : stage->m_reads) {
        const auto *texture_resource = resource.first->as<TextureResource>();
        if (texture_resource == nullptr) {
            continue;
        }
        auto *physical_texture = texture_resource->m_physical->as<PhysicalImage>();
        if (physical_texture->m_img == nullptr) {
            continue;
        }
        if (texture_resource->m_usage == TextureUsage::DEPTH_STENCIL_BUFFER) {
            depth_buffer_img_view = physical_texture->image_view();
            depth_buffer = physical_texture->m_img->image();
        }
    }

    const auto depth_attachment = wrapper::make_info<VkRenderingAttachmentInfo>({
        .imageView = depth_buffer_img_view,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .loadOp = graphics_stage->m_clears_screen ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue =
            {
                .depthStencil = graphics_stage->m_clear_value.depthStencil,
            },
    });

    const auto rendering_info = wrapper::make_info<VkRenderingInfo>({
        .renderArea =
            {
                .extent = m_swapchain.extent(),
            },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
        .pDepthAttachment = &depth_attachment,
        .pStencilAttachment = &depth_attachment,
    });

    cmd_buf.begin_rendering(&rendering_info);

    // TODO: Reserve memory here?
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

    // TODO: Can/should we batch push constant ranges into one(?)
    for (auto &push_constant : stage->m_push_constants) {
        cmd_buf.push_constants(physical.m_pipeline_layout->pipeline_layout(), push_constant.m_push_constant.stageFlags,
                               push_constant.m_push_constant.size, push_constant.m_push_constant_data);
    }

    cmd_buf.bind_descriptor_set(physical.m_descriptor_set, physical.m_pipeline_layout->pipeline_layout());

    // Call the recording function (the custom command buffer code) that was specified by the programmer for this stage
    stage->m_on_record(cmd_buf);

    cmd_buf.end_rendering();

    if (last_stage) {
        float color[4];
        color[0] = 0.0f;
        color[1] = 1.0f;
        color[2] = 0.0f;
        color[3] = 0.4f;
        cmd_buf.insert_debug_label("Hello world", color);
        cmd_buf.change_image_layout(m_swapchain.image(image_index), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    cmd_buf.end_debug_label_region();
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
                VK_IMAGE_LAYOUT_UNDEFINED, texture_resource->name());
        }
    }
}

void RenderGraph::build_descriptor_sets(const RenderStage *stage) {
    // Use the descriptor builder to assemble the descriptor
    for (auto &read_resource : stage->m_reads) {
        // For simplicity reasons, check if it's an external texture resource first
        auto *external_texture = read_resource.first->as<ExternalTextureResource>();
        if (external_texture != nullptr) {
            // Add combined image sampler to the descriptor set layout builder
            m_descriptor_set_layout_builder.add_combined_image_sampler(read_resource.second.value());
        }
        auto *physical = read_resource.first->m_physical->as<PhysicalBuffer>();
        if (physical != nullptr) {
            // This is a buffer, so check if it's a uniform buffer
            auto *buffer = read_resource.first->as<BufferResource>();
            if (buffer != nullptr) {
                if (buffer->m_usage == BufferUsage::UNIFORM_BUFFER) {
                    // Add uniform buffer to the descriptor set layout builder
                    m_descriptor_set_layout_builder.add_uniform_buffer(read_resource.second.value());
                }
            }
        }
    }
    // Build the descriptor set layout
    const auto descriptor_set_layout = m_descriptor_set_layout_builder.build();
    stage->m_physical->m_descriptor_set_layout = descriptor_set_layout;
    // Allocate the descriptor set using the descriptor set allocator
    stage->m_physical->m_descriptor_set = m_descriptor_set_allocator.allocate_descriptor_set(descriptor_set_layout);
}

void RenderGraph::create_push_constant_ranges(GraphicsStage *stage) {
    // Collect the push constant ranges of this stage into one std::vector
    stage->m_push_constant_ranges.reserve(stage->m_push_constants.size());
    for (const auto &push_constant : stage->m_push_constants) {
        stage->m_push_constant_ranges.push_back(push_constant.m_push_constant);
    }
}

void RenderGraph::create_pipeline_layout(PhysicalStage &physical, GraphicsStage *stage) {
    physical.m_pipeline_layout = std::make_unique<wrapper::pipelines::PipelineLayout>(
        m_device, std::vector{stage->m_physical->m_descriptor_set_layout}, stage->m_push_constant_ranges,
        "Graphics Pipeline Layout " + stage->name());
}

void RenderGraph::create_graphics_pipeline(PhysicalStage &physical, GraphicsStage *stage) {
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
            ->set_scissor(m_swapchain.extent())
            ->set_viewport(m_swapchain.extent())
            ->make_create_info(m_swapchain.image_format()),
        "Graphics Pipeline " + stage->name());
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

void RenderGraph::collect_render_stages_reading_from_uniform_buffers() {
    m_log->trace("Connecting render stages to render resources");

    // Here we sacrifice a little more memory for the sake of performance
    m_uniform_buffer_reading_stages.resize(m_buffer_resources.size());

    // First loop through all buffer resources and store their index in the m_buffer_resources vector
    for (std::size_t index = 0; index < m_buffer_resources.size(); index++) {
        m_buffer_resources[index]->m_my_buffer_index = index;
    }
    // Now loop through all stages and analyze which stage is reading from which uniform buffer
    for (auto &stage : m_stage_stack) {
        for (auto &render_resource : stage->m_reads) {
            auto buffer_resource = render_resource.first->as<BufferResource>();
            // Check if the dynamic cast has worked
            if (buffer_resource != nullptr) {
                // Check if this is a uniform buffer
                if (buffer_resource->m_usage == BufferUsage::UNIFORM_BUFFER) {
                    // Remember that this uniform buffer is read by this stage
                    m_uniform_buffer_reading_stages[buffer_resource->m_my_buffer_index].push_back(stage);
                    m_log->trace("   - Stage '{}' is reading from uniform buffer '{}' [buffer resource index {}]",
                                 stage->m_name, buffer_resource->name(), buffer_resource->m_my_buffer_index);
                }
            }
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

    // Create physical stages
    //  - Each render stage maps to a vulkan pipeline (either compute or graphics) and a list of command buffers
    //  - Each graphics stage also maps to one vulkan render pass
    for (auto *stage : m_stage_stack) {
        if (auto *graphics_stage = stage->as<GraphicsStage>()) {
            // TODO: Can't we simplify this?
            auto physical_ptr = std::make_unique<PhysicalStage>(m_device);
            auto &physical = *physical_ptr;
            graphics_stage->m_physical = std::move(physical_ptr);

            build_descriptor_sets(graphics_stage);
            create_push_constant_ranges(graphics_stage);
            create_pipeline_layout(physical, graphics_stage);
            create_graphics_pipeline(physical, graphics_stage);
        }
    }
    collect_render_stages_reading_from_uniform_buffers();
    update_uniform_buffer_descriptor_sets();
    update_texture_descriptor_sets();
}

void RenderGraph::update_texture_descriptor_sets() {
    // Loop through all stages
    for (const auto &stage : m_stage_stack) {
        // Go through all external texture resources
        for (auto &read_resource : stage->m_reads) {
            auto *external_texture = read_resource.first->as<ExternalTextureResource>();
            if (external_texture != nullptr) {
                external_texture->m_descriptor_image_info = VkDescriptorImageInfo{
                    .sampler = external_texture->m_texture.sampler(),
                    .imageView = external_texture->m_texture.image_view(),
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                };
                // Add the combined image sampler to the descriptor set update builder
                m_descriptor_set_updater.add_combined_image_sampler_update(stage->m_physical->m_descriptor_set,
                                                                           &external_texture->m_descriptor_image_info);
            }
        }
    }
    m_descriptor_set_updater.update_descriptor_sets();
}

void RenderGraph::update_uniform_buffer_descriptor_sets() {
    // Loop through all indices of updated uniform buffer resources
    for (auto &index_of_updated_buffer : m_indices_of_updated_uniform_buffers) {
        // Now for that uniform buffer, get all the render stages which read from it
        for (const auto &render_stage : m_uniform_buffer_reading_stages[index_of_updated_buffer]) {
            // Add this uniform buffer update to the descriptor set update builder
            m_descriptor_set_updater.add_uniform_buffer_update(
                // TODO: We have a vector of descriptor sets per stage, but yet we only use and update index 0
                render_stage->m_physical->m_descriptor_set,
                // The descriptor buffer info has already been updated in update_dynamic_buffers()
                &m_buffer_resources[index_of_updated_buffer]->m_physical_buffer->m_descriptor_buffer_info);
        }
    }
    // Note that we batch all descriptor set updates into one call to vkUpdateDescriptorSets for performance reasons
    m_descriptor_set_updater.update_descriptor_sets();
    // All descriptor sets have been updated
    m_indices_of_updated_uniform_buffers.clear();
}

void RenderGraph::update_push_constant_ranges() {
    for (const auto &stage : m_stage_stack) {
        stage->m_on_update();
        for (auto &push_constant : stage->m_push_constants) {
            push_constant.m_on_update();
        }
    }
}

void RenderGraph::create_buffer(PhysicalBuffer &physical, BufferResource *buffer_resource) {
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

    // Let's just store a pointer from the buffer resource to the physical buffer
    // TODO: We should not do this in the future!
    buffer_resource->m_physical_buffer = &physical;
}

void RenderGraph::update_dynamic_buffers() {
    for (std::size_t index = 0; index < m_buffer_resources.size(); index++) {
        auto &buffer_resource = m_buffer_resources[index];
        auto &physical = *buffer_resource->m_physical->as<PhysicalBuffer>();

        // Call the buffer's update function
        buffer_resource->m_on_update();

        if (buffer_resource->m_data_upload_needed) {
            // Check if this buffer has already been created
            if (physical.m_buffer != nullptr) {
                // TODO: Implement a recreate() command (don't reset the unique ptr!)
                // physical.m_buffer->recreate(..);
                physical.m_buffer.reset();
                physical.m_buffer = nullptr;
            }
            // TODO: Should we check if the size is smaller than the current size and not recreate?
            // TODO: When implementing .recreate, move the line below to an else {} block!
            create_buffer(physical, buffer_resource.get());

            // If it's a uniform buffer, we need to update descriptors!
            if (buffer_resource->m_usage == BufferUsage::UNIFORM_BUFFER) {
                // Remember that this uniform buffer has been updated
                m_indices_of_updated_uniform_buffers.push_back(index);

                // TODO: Wait a minute... do we really even need this here?
                // Update the descriptor buffer info
                buffer_resource->m_physical_buffer->m_descriptor_buffer_info = VkDescriptorBufferInfo{
                    .buffer = physical.m_buffer->buffer(),
                    .offset = 0,
                    .range = buffer_resource->m_data_size, // TODO: Is this correct?
                };
            }
            // TODO: Implement updates which requires staging buffers!
            std::memcpy(physical.m_buffer->memory(), buffer_resource->m_data, buffer_resource->m_data_size);
        }
    }
}

void RenderGraph::render(const std::uint32_t image_index, const wrapper::CommandBuffer &cmd_buf) {
    // TODO: Updating push constant ranges can be done in parallel using taskflow library
    update_push_constant_ranges();
    // TODO: Updating dynamic buffers can be done in parallel using taskflow library
    update_dynamic_buffers();
    // TODO: Updating both the dynamic buffers and push constant range can be done at the same time
    // Everything must have finished updating before we can update descriptor sets
    update_uniform_buffer_descriptor_sets();
    // TODO: update_texture_descriptor_sets

    // TODO: Command buffer recording can be done in parallel using taskflow library
    for (std::size_t stage_index = 0; stage_index < m_stage_stack.size(); stage_index++) {
        record_command_buffer(stage_index == 0, stage_index == (m_stage_stack.size() - 1), m_stage_stack[stage_index],
                              cmd_buf, image_index);
    }
}

} // namespace inexor::vulkan_renderer
