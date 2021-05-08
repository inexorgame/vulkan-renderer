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

void RenderStage::writes_to(const RenderResource &resource) {
    m_writes.push_back(&resource);
}

void RenderStage::reads_from(const RenderResource &resource) {
    m_reads.push_back(&resource);
}

void GraphicsStage::bind_buffer(const BufferResource &buffer, std::uint32_t binding) {
    m_buffer_bindings.emplace(&buffer, binding);
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

void RenderGraph::build_image(const TextureResource *resource, PhysicalImage *phys,
                              VmaAllocationCreateInfo *alloc_ci) const {
    auto image_ci = wrapper::make_info<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;

    // TODO: Support textures with dimensions not equal to back buffer size.
    image_ci.extent.width = m_swapchain.extent().width;
    image_ci.extent.height = m_swapchain.extent().height;
    image_ci.extent.depth = 1;

    image_ci.arrayLayers = 1;
    image_ci.mipLevels = 1;
    image_ci.format = resource->m_format;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = resource->m_usage == TextureUsage::DEPTH_STENCIL_BUFFER
                         ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                         : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VmaAllocationInfo alloc_info;
    if (const auto result =
            vmaCreateImage(m_device.allocator(), &image_ci, alloc_ci, &phys->m_image, &phys->m_allocation, &alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create image!", result);
    }
}

void RenderGraph::build_image_view(const TextureResource *resource, PhysicalImage *phys) const {
    auto image_view_ci = wrapper::make_info<VkImageViewCreateInfo>();
    image_view_ci.format = resource->m_format;
    image_view_ci.image = phys->m_image;
    image_view_ci.subresourceRange.aspectMask = resource->m_usage == TextureUsage::DEPTH_STENCIL_BUFFER
                                                    ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
                                                    : VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.layerCount = 1;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;

    if (const auto result = vkCreateImageView(m_device.device(), &image_view_ci, nullptr, &phys->m_image_view);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create image view!", result);
    }
}

void RenderGraph::alloc_command_buffers(const RenderStage *stage, PhysicalStage *phys) const {
    m_log->trace("Allocating command buffers for stage '{}'", stage->m_name);
    for (std::uint32_t i = 0; i < m_swapchain.image_count(); i++) {
        phys->m_command_buffers.emplace_back(m_device, m_command_pool, "Command buffer for stage " + stage->m_name);
    }
}

void RenderGraph::build_pipeline_layout(const RenderStage *stage, PhysicalStage *phys) const {
    auto pipeline_layout_ci = wrapper::make_info<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = static_cast<std::uint32_t>(stage->m_descriptor_layouts.size());
    pipeline_layout_ci.pSetLayouts = stage->m_descriptor_layouts.data();
    if (const auto result =
            vkCreatePipelineLayout(m_device.device(), &pipeline_layout_ci, nullptr, &phys->m_pipeline_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create pipeline layout!", result);
    }

    m_device.set_debug_marker_name(phys->m_pipeline_layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,
                                   stage->m_name + " pipeline layout");
}

void RenderGraph::record_command_buffers(const RenderStage *stage, PhysicalStage *phys) const {
    for (std::size_t i = 0; i < phys->m_command_buffers.size(); i++) {
        // TODO: Remove simultaneous usage once we have proper max frames in flight control.
        auto &cmd_buf = phys->m_command_buffers[i];
        cmd_buf.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        // Record render pass for graphics stages.
        const auto *graphics_stage = stage->as<GraphicsStage>();
        if (graphics_stage != nullptr) {
            const auto *phys_graphics_stage = phys->as<PhysicalGraphicsStage>();
            assert(phys_graphics_stage != nullptr);

            auto render_pass_bi = wrapper::make_info<VkRenderPassBeginInfo>();
            std::array<VkClearValue, 2> clear_values{};
            if (graphics_stage->m_clears_screen) {
                clear_values[0].color = {0, 0, 0, 0};
                clear_values[1].depthStencil = {1.0f, 0};
                render_pass_bi.clearValueCount = static_cast<std::uint32_t>(clear_values.size());
                render_pass_bi.pClearValues = clear_values.data();
            }
            render_pass_bi.framebuffer = phys_graphics_stage->m_framebuffers[i].get();
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

            const auto *phys_buffer = m_resource_map.at(resource)->as<PhysicalBuffer>();
            assert(phys_buffer != nullptr);

            if (buffer_resource->m_usage == BufferUsage::INDEX_BUFFER) {
                cmd_buf.bind_index_buffer(phys_buffer->m_buffer);
            } else if (buffer_resource->m_usage == BufferUsage::VERTEX_BUFFER) {
                vertex_buffers.push_back(phys_buffer->m_buffer);
            }
        }

        if (!vertex_buffers.empty()) {
            cmd_buf.bind_vertex_buffers(vertex_buffers);
        }

        cmd_buf.bind_graphics_pipeline(phys->m_pipeline);
        stage->m_on_record(phys, cmd_buf);

        if (graphics_stage != nullptr) {
            cmd_buf.end_render_pass();
        }
        cmd_buf.end();
    }
}

void RenderGraph::build_render_pass(const GraphicsStage *stage, PhysicalGraphicsStage *phys) const {
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
    subpass_description.pDepthStencilAttachment = depth_refs.data();
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    auto render_pass_ci = wrapper::make_info<VkRenderPassCreateInfo>();
    render_pass_ci.attachmentCount = static_cast<std::uint32_t>(attachments.size());
    render_pass_ci.dependencyCount = 1;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pAttachments = attachments.data();
    render_pass_ci.pDependencies = &subpass_dependency;
    render_pass_ci.pSubpasses = &subpass_description;
    if (const auto result = vkCreateRenderPass(m_device.device(), &render_pass_ci, nullptr, &phys->m_render_pass);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create render pass!", result);
    }
}

void RenderGraph::build_graphics_pipeline(const GraphicsStage *stage, PhysicalGraphicsStage *phys) const {
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

    // TODO: Allow depth testing to be disabled.
    // TODO: Also allow depth compare func to be changed?
    auto depth_stencil = wrapper::make_info<VkPipelineDepthStencilStateCreateInfo>();
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;

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

    VkPipelineColorBlendAttachmentState blend_attachment{};
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
    pipeline_ci.layout = phys->m_pipeline_layout;
    pipeline_ci.renderPass = phys->m_render_pass;
    pipeline_ci.stageCount = static_cast<std::uint32_t>(stage->m_shaders.size());
    pipeline_ci.pStages = stage->m_shaders.data();

    // TODO: Pipeline caching (basically load the render graph from a file)
    if (const auto result =
            vkCreateGraphicsPipelines(m_device.device(), nullptr, 1, &pipeline_ci, nullptr, &phys->m_pipeline);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create pipeline!", result);
    }
}

void RenderGraph::compile(const RenderResource &target) {
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
    for (auto *stage : writers[&target]) {
        dfs(stage);
    }

    m_log->debug("Final stage order:");
    for (auto *stage : m_stage_stack) {
        m_log->debug("  - {}", stage->m_name);
    }

    // Create physical resources. For now, each buffer or texture resource maps directly to either a VkBuffer or VkImage
    // respectively. Every physical resource also has a VmaAllocation.
    // TODO: Resource aliasing (i.e. reusing the same physical resource for multiple resources).
    for (const auto &resource : m_resources) {
        // Build allocation (using VMA for now).
        m_log->trace("Allocating physical resource for resource '{}'", resource->m_name);
        VmaAllocationCreateInfo alloc_ci{};

        // TODO: Use a constexpr bool.
#if VMA_RECORDING_ENABLED
        alloc_ci.flags |= VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        alloc_ci.pUserData = const_cast<char *>(resource->m_name.data());
#endif

        if (const auto *buffer_resource = resource->as<BufferResource>()) {
            auto *phys = create<PhysicalBuffer>(buffer_resource, m_device.allocator(), m_device.device());

            const bool is_uploading_data = buffer_resource->m_data != nullptr;
            alloc_ci.flags |= is_uploading_data ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0u;
            alloc_ci.usage = is_uploading_data ? VMA_MEMORY_USAGE_CPU_TO_GPU : VMA_MEMORY_USAGE_GPU_ONLY;

            auto buffer_ci = wrapper::make_info<VkBufferCreateInfo>();
            buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buffer_ci.size = buffer_resource->m_data_size;
            switch (buffer_resource->m_usage) {
            case BufferUsage::INDEX_BUFFER:
                buffer_ci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
                break;
            case BufferUsage::VERTEX_BUFFER:
                buffer_ci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
                break;
            default:
                assert(false);
            }

            VmaAllocationInfo alloc_info;
            if (const auto result = vmaCreateBuffer(m_device.allocator(), &buffer_ci, &alloc_ci, &phys->m_buffer,
                                                    &phys->m_allocation, &alloc_info);
                result != VK_SUCCESS) {
                throw VulkanException("Failed to create buffer!", result);
            }

            if (is_uploading_data) {
                assert(alloc_info.pMappedData != nullptr);
                std::memcpy(alloc_info.pMappedData, buffer_resource->m_data, buffer_resource->m_data_size);
            }
        }

        if (const auto *texture_resource = resource->as<TextureResource>()) {
            // Back buffer gets special handling.
            if (texture_resource->m_usage == TextureUsage::BACK_BUFFER) {
                // TODO: Move image views from wrapper::Swapchain to PhysicalBackBuffer.
                create<PhysicalBackBuffer>(texture_resource, m_device.allocator(), m_device.device(), m_swapchain);
            } else {
                auto *phys = create<PhysicalImage>(texture_resource, m_device.allocator(), m_device.device());
                alloc_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
                build_image(texture_resource, phys, &alloc_ci);
                build_image_view(texture_resource, phys);
            }
        }
    }

    // Create physical stages. Each render stage maps to a vulkan pipeline (either compute or graphics) and a list of
    // command buffers. Each graphics stage also maps to a vulkan render pass.
    for (const auto *stage : m_stage_stack) {
        if (const auto *graphics_stage = stage->as<GraphicsStage>()) {
            auto *phys = create<PhysicalGraphicsStage>(graphics_stage, m_device);
            build_render_pass(graphics_stage, phys);
            build_pipeline_layout(graphics_stage, phys);
            build_graphics_pipeline(graphics_stage, phys);

            // If we write to at least one texture, we need to make framebuffers.
            if (!stage->m_writes.empty()) {
                // For every texture that this stage writes to, we need to attach it to the framebuffer.
                std::vector<const PhysicalBackBuffer *> back_buffers;
                std::vector<const PhysicalImage *> images;
                for (const auto *resource : stage->m_writes) {
                    const auto *phys_resource = m_resource_map[resource].get();
                    if (const auto *back_buffer = phys_resource->as<PhysicalBackBuffer>()) {
                        back_buffers.push_back(back_buffer);
                    } else if (const auto *image = phys_resource->as<PhysicalImage>()) {
                        images.push_back(image);
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

                    phys->m_framebuffers.emplace_back(m_device, phys->m_render_pass, image_views, m_swapchain,
                                                      "Framebuffer");
                }
            }
        }
    }

    // Allocate and record command buffers.
    for (const auto *stage : m_stage_stack) {
        auto *phys = m_stage_map[stage].get();
        alloc_command_buffers(stage, phys);
        record_command_buffers(stage, phys);
    }
}

void RenderGraph::render(int image_index, VkFence signal_fence, VkSemaphore signal_semaphore,
                         VkSemaphore wait_semaphore, VkQueue graphics_queue) const {
    auto submit_info = wrapper::make_info<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.signalSemaphoreCount = 1;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &signal_semaphore;
    submit_info.pWaitSemaphores = &wait_semaphore;

    std::array<VkPipelineStageFlags, 1> wait_stage_mask = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = wait_stage_mask.data();

    // TODO: Batch submit infos.
    for (const auto *stage : m_phys_stage_stack) {
        auto *cmd_buf = stage->m_command_buffers[image_index].get();
        submit_info.pCommandBuffers = &cmd_buf;
        vkQueueSubmit(graphics_queue, 1, &submit_info, signal_fence);
    }
}

} // namespace inexor::vulkan_renderer
