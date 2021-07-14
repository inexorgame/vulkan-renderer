#include "inexor/vulkan-renderer/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <array>
#include <cassert>
#include <functional>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace inexor::vulkan_renderer {

void BufferResource::add_vertex_attribute(VkFormat format, std::uint32_t offset) {
    VkVertexInputAttributeDescription vertex_attribute{};
    vertex_attribute.format = format;
    vertex_attribute.location = static_cast<std::uint32_t>(m_vertex_attributes.size());
    vertex_attribute.offset = offset;
    m_vertex_attributes.push_back(vertex_attribute);
}

void RenderStage::writes_to(const RenderResource *resource) {
    m_writes.push_back(resource);
}

void RenderStage::reads_from(const RenderResource *resource) {
    m_reads.push_back(resource);
}

void GraphicsStage::bind_buffer(const BufferResource *buffer, const std::uint32_t binding) {
    m_buffer_bindings.emplace(buffer, binding);
}

void GraphicsStage::uses_shader(const wrapper::Shader &shader) {
    auto create_info = wrapper::make_info<VkPipelineShaderStageCreateInfo>();
    create_info.module = shader.module();
    create_info.stage = shader.type();
    create_info.pName = shader.entry_point().c_str();
    m_shaders.push_back(create_info);
}

PhysicalBuffer::~PhysicalBuffer() {
    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
}

PhysicalImage::~PhysicalImage() {
    vkDestroyImageView(m_device, m_image_view, nullptr);
    vmaDestroyImage(m_allocator, m_image, m_allocation);
}

PhysicalStage::~PhysicalStage() {
    vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device.device(), m_pipeline_layout, nullptr);
}

PhysicalGraphicsStage::~PhysicalGraphicsStage() {
    vkDestroyRenderPass(device(), m_render_pass, nullptr);
}

void RenderGraph::build_buffer(const BufferResource &buffer_resource, PhysicalBuffer &physical) const {
    // TODO: Don't always create mapped.
    VmaAllocationCreateInfo alloc_ci{};
    alloc_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    alloc_ci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    auto buffer_ci = wrapper::make_info<VkBufferCreateInfo>();
    buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_ci.size = buffer_resource.m_data_size;
    switch (buffer_resource.m_usage) {
    case BufferUsage::INDEX_BUFFER:
        buffer_ci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        break;
    case BufferUsage::VERTEX_BUFFER:
        buffer_ci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        break;
    default:
        assert(false);
    }

    VmaAllocationInfo &alloc_info = physical.m_alloc_info;
    if (const auto result = vmaCreateBuffer(m_device.allocator(), &buffer_ci, &alloc_ci, &physical.m_buffer,
                                            &physical.m_allocation, &alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create buffer!", result);
    }
}

void RenderGraph::build_image(const TextureResource &texture_resource, PhysicalImage &physical,
                              VmaAllocationCreateInfo *alloc_ci) const {
    auto image_ci = wrapper::make_info<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;

    // TODO: Support textures with dimensions not equal to back buffer size.
    image_ci.extent.width = m_swapchain.extent().width;
    image_ci.extent.height = m_swapchain.extent().height;
    image_ci.extent.depth = 1;

    image_ci.arrayLayers = 1;
    image_ci.mipLevels = 1;
    image_ci.format = texture_resource.m_format;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = texture_resource.m_usage == TextureUsage::DEPTH_STENCIL_BUFFER
                         ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                         : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VmaAllocationInfo alloc_info;
    if (const auto result = vmaCreateImage(m_device.allocator(), &image_ci, alloc_ci, &physical.m_image,
                                           &physical.m_allocation, &alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create image!", result);
    }
}

void RenderGraph::build_image_view(const TextureResource &texture_resource, PhysicalImage &physical) const {
    auto image_view_ci = wrapper::make_info<VkImageViewCreateInfo>();
    image_view_ci.format = texture_resource.m_format;
    image_view_ci.image = physical.m_image;
    image_view_ci.subresourceRange.aspectMask = texture_resource.m_usage == TextureUsage::DEPTH_STENCIL_BUFFER
                                                    ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
                                                    : VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.layerCount = 1;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;

    if (const auto result = vkCreateImageView(m_device.device(), &image_view_ci, nullptr, &physical.m_image_view);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create image view!", result);
    }
}

void RenderGraph::build_pipeline_layout(const RenderStage *stage, PhysicalStage &physical) const {
    auto pipeline_layout_ci = wrapper::make_info<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = static_cast<std::uint32_t>(stage->m_descriptor_layouts.size());
    pipeline_layout_ci.pSetLayouts = stage->m_descriptor_layouts.data();
    pipeline_layout_ci.pushConstantRangeCount = static_cast<std::uint32_t>(stage->m_push_constant_ranges.size());
    pipeline_layout_ci.pPushConstantRanges = stage->m_push_constant_ranges.data();
    if (const auto result =
            vkCreatePipelineLayout(m_device.device(), &pipeline_layout_ci, nullptr, &physical.m_pipeline_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create pipeline layout!", result);
    }

    m_device.set_debug_marker_name(physical.m_pipeline_layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,
                                   stage->m_name + " pipeline layout");
}

void RenderGraph::record_command_buffer(const RenderStage *stage, PhysicalStage &physical,
                                        const std::uint32_t image_index) const {
    auto &cmd_buf = physical.m_command_buffers[image_index];
    cmd_buf.begin();

    // Record render pass for graphics stages.
    const auto *graphics_stage = stage->as<GraphicsStage>();
    if (graphics_stage != nullptr) {
        const auto *phys_graphics_stage = physical.as<PhysicalGraphicsStage>();
        assert(phys_graphics_stage != nullptr);

        auto render_pass_bi = wrapper::make_info<VkRenderPassBeginInfo>();
        std::array<VkClearValue, 2> clear_values{};
        if (graphics_stage->m_clears_screen) {
            clear_values[0].color = {0, 0, 0, 0};
            clear_values[1].depthStencil = {1.0f, 0};
            render_pass_bi.clearValueCount = static_cast<std::uint32_t>(clear_values.size());
            render_pass_bi.pClearValues = clear_values.data();
        }
        render_pass_bi.framebuffer = phys_graphics_stage->m_framebuffers[image_index].get();
        render_pass_bi.renderArea.extent = m_swapchain.extent();
        render_pass_bi.renderPass = phys_graphics_stage->m_render_pass;
        cmd_buf.begin_render_pass(render_pass_bi);
    }

    std::vector<VkBuffer> vertex_buffers;
    for (const auto *resource : stage->m_reads) {
        const auto *buffer_resource = resource->as<BufferResource>();
        if (buffer_resource == nullptr) {
            continue;
        }

        auto *physical_buffer = buffer_resource->m_physical->as<PhysicalBuffer>();
        if (physical_buffer->m_buffer == nullptr) {
            continue;
        }
        if (buffer_resource->m_usage == BufferUsage::INDEX_BUFFER) {
            cmd_buf.bind_index_buffer(physical_buffer->m_buffer);
        } else if (buffer_resource->m_usage == BufferUsage::VERTEX_BUFFER) {
            vertex_buffers.push_back(physical_buffer->m_buffer);
        }
    }

    if (!vertex_buffers.empty()) {
        cmd_buf.bind_vertex_buffers(vertex_buffers);
    }

    cmd_buf.bind_graphics_pipeline(physical.m_pipeline);
    stage->m_on_record(physical, cmd_buf);

    if (graphics_stage != nullptr) {
        cmd_buf.end_render_pass();
    }
    cmd_buf.end();
}

void RenderGraph::build_render_pass(const GraphicsStage *stage, PhysicalGraphicsStage &physical) const {
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> colour_refs;
    std::vector<VkAttachmentReference> depth_refs;

    // Build vulkan attachments. For every texture resource that stage writes to, we create a corresponding
    // VkAttachmentDescription and attach it to the render pass.
    // TODO(GH-203): Support multisampled attachments.
    // TODO: Use range-based for loop initialization statements when we switch to C++ 20.
    for (std::size_t i = 0; i < stage->m_writes.size(); i++) {
        const auto *resource = stage->m_writes[i];
        const auto *texture = resource->as<TextureResource>();
        if (texture == nullptr) {
            continue;
        }

        VkAttachmentDescription attachment{};
        attachment.format = texture->m_format;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = stage->m_clears_screen ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        switch (texture->m_usage) {
        case TextureUsage::BACK_BUFFER:
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

    // Build a simple subpass that just waits for the output colour vector to be written by the fragment shader. In the
    // future, we may want to make use of subpasses more.
    VkSubpassDependency subpass_dependency{};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubpassDescription subpass_description{};
    subpass_description.colorAttachmentCount = static_cast<std::uint32_t>(colour_refs.size());
    subpass_description.pColorAttachments = colour_refs.data();
    subpass_description.pDepthStencilAttachment = !depth_refs.empty() ? depth_refs.data() : nullptr;
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    auto render_pass_ci = wrapper::make_info<VkRenderPassCreateInfo>();
    render_pass_ci.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    render_pass_ci.dependencyCount = 1;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pAttachments = attachments.data();
    render_pass_ci.pDependencies = &subpass_dependency;
    render_pass_ci.pSubpasses = &subpass_description;
    if (const auto result = vkCreateRenderPass(m_device.device(), &render_pass_ci, nullptr, &physical.m_render_pass);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create render pass!", result);
    }
}

void RenderGraph::build_graphics_pipeline(const GraphicsStage *stage, PhysicalGraphicsStage &physical) const {
    // Build buffer and vertex layout bindings. For every buffer resource that stage reads from, we create a
    // corresponding attribute binding and vertex binding description.
    std::vector<VkVertexInputAttributeDescription> attribute_bindings;
    std::vector<VkVertexInputBindingDescription> vertex_bindings;
    for (const auto *resource : stage->m_reads) {
        const auto *buffer_resource = resource->as<BufferResource>();
        if (buffer_resource == nullptr) {
            continue;
        }

        // Don't mess with index buffers here.
        if (buffer_resource->m_usage == BufferUsage::INDEX_BUFFER) {
            continue;
        }

        // We use std::unordered_map::at() here to ensure that a binding value exists for buffer_resource.
        const std::uint32_t binding = stage->m_buffer_bindings.at(buffer_resource);
        for (auto attribute_binding : buffer_resource->m_vertex_attributes) {
            attribute_binding.binding = binding;
            attribute_bindings.push_back(attribute_binding);
        }

        VkVertexInputBindingDescription vertex_binding{};
        vertex_binding.binding = binding;
        vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        vertex_binding.stride = static_cast<std::uint32_t>(buffer_resource->m_element_size);
        vertex_bindings.push_back(vertex_binding);
    }

    auto vertex_input = wrapper::make_info<VkPipelineVertexInputStateCreateInfo>();
    vertex_input.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attribute_bindings.size());
    vertex_input.vertexBindingDescriptionCount = static_cast<std::uint32_t>(vertex_bindings.size());
    vertex_input.pVertexAttributeDescriptions = attribute_bindings.data();
    vertex_input.pVertexBindingDescriptions = vertex_bindings.data();

    // TODO: Support primitives other than triangles.
    auto input_assembly = wrapper::make_info<VkPipelineInputAssemblyStateCreateInfo>();
    input_assembly.primitiveRestartEnable = VK_FALSE;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // TODO: Also allow depth compare func to be changed?
    auto depth_stencil = wrapper::make_info<VkPipelineDepthStencilStateCreateInfo>();
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil.depthTestEnable = stage->m_depth_test ? VK_TRUE : VK_FALSE;
    depth_stencil.depthWriteEnable = stage->m_depth_write ? VK_TRUE : VK_FALSE;

    // TODO: Allow culling to be disabled.
    // TODO: Wireframe rendering.
    auto rasterization_state = wrapper::make_info<VkPipelineRasterizationStateCreateInfo>();
    rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization_state.lineWidth = 1.0f;
    rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;

    // TODO(GH-203): Support multisampling again.
    auto multisample_state = wrapper::make_info<VkPipelineMultisampleStateCreateInfo>();
    multisample_state.minSampleShading = 1.0f;
    multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState blend_attachment = stage->m_blend_attachment;
    blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    auto blend_state = wrapper::make_info<VkPipelineColorBlendStateCreateInfo>();
    blend_state.attachmentCount = 1;
    blend_state.pAttachments = &blend_attachment;

    VkRect2D scissor{};
    scissor.extent = m_swapchain.extent();

    VkViewport viewport{};
    viewport.width = static_cast<float>(m_swapchain.extent().width);
    viewport.height = static_cast<float>(m_swapchain.extent().height);
    viewport.maxDepth = 1.0f;

    // TODO: Custom scissors?
    auto viewport_state = wrapper::make_info<VkPipelineViewportStateCreateInfo>();
    viewport_state.scissorCount = 1;
    viewport_state.viewportCount = 1;
    viewport_state.pScissors = &scissor;
    viewport_state.pViewports = &viewport;

    auto pipeline_ci = wrapper::make_info<VkGraphicsPipelineCreateInfo>();
    pipeline_ci.pVertexInputState = &vertex_input;
    pipeline_ci.pInputAssemblyState = &input_assembly;
    pipeline_ci.pDepthStencilState = &depth_stencil;
    pipeline_ci.pRasterizationState = &rasterization_state;
    pipeline_ci.pMultisampleState = &multisample_state;
    pipeline_ci.pColorBlendState = &blend_state;
    pipeline_ci.pViewportState = &viewport_state;
    pipeline_ci.layout = physical.m_pipeline_layout;
    pipeline_ci.renderPass = physical.m_render_pass;
    pipeline_ci.stageCount = static_cast<std::uint32_t>(stage->m_shaders.size());
    pipeline_ci.pStages = stage->m_shaders.data();

    // TODO: Pipeline caching (basically load the render graph from a file)
    if (const auto result =
            vkCreateGraphicsPipelines(m_device.device(), nullptr, 1, &pipeline_ci, nullptr, &physical.m_pipeline);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create pipeline!", result);
    }
}

void RenderGraph::compile(const RenderResource *target) {
    // TODO(GH-204): Better logging and input validation.
    // TODO: Many opportunities for optimisation.

    // Build a simple helper map to lookup a resource's writers.
    std::unordered_map<const RenderResource *, std::vector<RenderStage *>> writers;
    for (auto &stage : m_stages) {
        for (const auto *resource : stage->m_writes) {
            writers[resource].push_back(stage.get());
        }
    }

    // Post order depth first search. Note that this doesn't do any colouring, so it only works on acyclic graphs.
    // TODO(GH-204): Stage graph validation (ensuring no cycles, etc.).
    // TODO: Move away from recursive dfs algo.
    std::function<void(RenderStage *)> dfs = [&](RenderStage *stage) {
        for (const auto *resource : stage->m_reads) {
            for (auto *writer : writers[resource]) {
                dfs(writer);
            }
        }
        m_stage_stack.push_back(stage);
    };

    // DFS starting from writers of target (initial stage executants).
    for (auto *stage : writers[target]) {
        dfs(stage);
    }

    m_log->debug("Final stage order:");
    for (auto *stage : m_stage_stack) {
        m_log->debug("  - {}", stage->m_name);
    }

    // Create physical resources. For now, each buffer or texture resource maps directly to either a VkBuffer or VkImage
    // respectively. Every physical resource also has a VmaAllocation.
    // TODO: Resource aliasing (i.e. reusing the same physical resource for multiple resources).
    for (auto &buffer_resource : m_buffer_resources) {
        m_log->trace("Allocating physical resource for buffer '{}'", buffer_resource->m_name);
        buffer_resource->m_physical = std::make_shared<PhysicalBuffer>(m_device.allocator(), m_device.device());
    }
    for (auto &texture_resource : m_texture_resources) {
        m_log->trace("Allocating physical resource for texture '{}'", texture_resource->m_name);
        // Back buffer gets special handling.
        if (texture_resource->m_usage == TextureUsage::BACK_BUFFER) {
            // TODO: Move image views from wrapper::Swapchain to PhysicalBackBuffer.
            texture_resource->m_physical =
                std::make_shared<PhysicalBackBuffer>(m_device.allocator(), m_device.device(), m_swapchain);
            continue;
        }

        // TODO: Use a constexpr bool.
        VmaAllocationCreateInfo alloc_ci{};
#if VMA_RECORDING_ENABLED
        alloc_ci.flags |= VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        alloc_ci.pUserData = const_cast<char *>(resource->m_name.data());
#endif

        auto physical = std::make_shared<PhysicalImage>(m_device.allocator(), m_device.device());
        texture_resource->m_physical = physical;
        alloc_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        build_image(*texture_resource, *physical, &alloc_ci);
        build_image_view(*texture_resource, *physical);
    }

    // Create physical stages. Each render stage maps to a vulkan pipeline (either compute or graphics) and a list of
    // command buffers. Each graphics stage also maps to a vulkan render pass.
    for (auto *stage : m_stage_stack) {
        if (auto *graphics_stage = stage->as<GraphicsStage>()) {
            auto physical_ptr = std::make_unique<PhysicalGraphicsStage>(m_device);
            auto &physical = *physical_ptr;
            graphics_stage->m_physical = std::move(physical_ptr);

            build_render_pass(graphics_stage, physical);
            build_pipeline_layout(graphics_stage, physical);
            build_graphics_pipeline(graphics_stage, physical);

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
                for (std::uint32_t i = 0; i < m_swapchain.image_count(); i++) {
                    image_views.clear();
                    for (const auto *back_buffer : back_buffers) {
                        image_views.push_back(back_buffer->m_swapchain.image_view(i));
                    }

                    for (const auto *image : images) {
                        image_views.push_back(image->m_image_view);
                    }

                    physical.m_framebuffers.emplace_back(m_device, physical.m_render_pass, image_views, m_swapchain,
                                                         "Framebuffer");
                }
            }
        }
    }

    // Allocate command buffers and finished semaphore.
    for (const auto *stage : m_stage_stack) {
        auto &physical = *stage->m_physical;
        physical.m_finished_semaphore =
            std::make_unique<wrapper::Semaphore>(m_device, "Finished semaphore for stage " + stage->m_name);
        m_log->trace("Allocating command buffers for stage '{}'", stage->m_name);
        for (std::uint32_t i = 0; i < m_swapchain.image_count(); i++) {
            physical.m_command_buffers.emplace_back(m_device, m_command_pool,
                                                    "Command buffer for stage " + stage->m_name);
        }
    }
}

VkSemaphore RenderGraph::render(std::uint32_t image_index, VkSemaphore wait_semaphore, VkQueue graphics_queue,
                                VkFence signal_fence) {
    // Update dynamic buffers.
    for (auto &buffer_resource : m_buffer_resources) {
        if (buffer_resource->m_data_upload_needed) {
            auto &physical = *buffer_resource->m_physical->as<PhysicalBuffer>();

            // Destroy the old buffer and create a new one with the required size.
            vmaDestroyBuffer(physical.m_allocator, physical.m_buffer, physical.m_allocation);
            build_buffer(*buffer_resource, physical);

            // Upload new data.
            assert(physical.m_alloc_info.pMappedData != nullptr);
            std::memcpy(physical.m_alloc_info.pMappedData, buffer_resource->m_data, buffer_resource->m_data_size);
            buffer_resource->m_data_upload_needed = false;
        }
    }

    // Re-record all command buffers. This isn't great, but it's fine for simple rendering pipelines. Eventually we'll
    // want to generate these in parallel.
    vkResetCommandPool(m_device.device(), m_command_pool, 0);
    for (const auto *stage : m_stage_stack) {
        record_command_buffer(stage, *stage->m_physical, image_index);
    }

    auto submit_info = wrapper::make_info<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.signalSemaphoreCount = 1;
    submit_info.waitSemaphoreCount = 1;

    std::array<VkPipelineStageFlags, 1> wait_stage_mask{
        // Wait on wait_semaphore before writing to the framebuffer.
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    submit_info.pWaitDstStageMask = wait_stage_mask.data();

    std::vector<VkSemaphore> wait_semaphores;
    for (const auto *stage : m_stage_stack) {
        wait_semaphores.push_back(wait_semaphore);
        wait_semaphore = stage->m_physical->m_finished_semaphore->get();
    }

    std::vector<VkSubmitInfo> submit_infos;
    for (std::size_t i = 0; i < m_stage_stack.size(); i++) {
        const auto &physical = *m_stage_stack[i]->m_physical;
        submit_info.pCommandBuffers = physical.m_command_buffers[image_index].ptr();
        submit_info.pSignalSemaphores = physical.m_finished_semaphore->ptr();
        submit_info.pWaitSemaphores = &wait_semaphores[i];
        submit_infos.push_back(submit_info);
    }
    vkQueueSubmit(graphics_queue, submit_infos.size(), submit_infos.data(), signal_fence);
    return m_stage_stack.back()->m_physical->m_finished_semaphore->get();
}

} // namespace inexor::vulkan_renderer
