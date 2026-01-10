#include "inexor/vulkan-renderer/render_graph.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>
#include <volk.h>

#include <array>
#include <cassert>
#include <functional>
#include <utility>

namespace inexor::vulkan_renderer {

using tools::VulkanException;

void BufferResource::add_vertex_attribute(VkFormat format, std::uint32_t offset) {
    m_vertex_attributes.push_back({
        .location = static_cast<std::uint32_t>(m_vertex_attributes.size()),
        .format = format,
        .offset = offset,
    });
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
    m_shaders.push_back(wrapper::make_info<VkPipelineShaderStageCreateInfo>({
        .stage = shader.shader_stage(),
        .module = shader.shader_module(),
        .pName = shader.entry_point().c_str(),
    }));
}

PhysicalBuffer::~PhysicalBuffer() {
    vmaDestroyBuffer(m_device.allocator(), m_buffer, m_allocation);
}

PhysicalImage::~PhysicalImage() {
    vkDestroyImageView(m_device.device(), m_image_view, nullptr);
    vmaDestroyImage(m_device.allocator(), m_image, m_allocation);
}

PhysicalGraphicsStage::~PhysicalGraphicsStage() {
    vkDestroyRenderPass(m_device.device(), m_render_pass, nullptr);
}

void RenderGraph::build_buffer(const BufferResource &buffer_resource, PhysicalBuffer &physical) const {
    // TODO: Don't always create mapped.
    const VmaAllocationCreateInfo alloc_ci{
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    };

    auto buffer_ci = wrapper::make_info<VkBufferCreateInfo>({
        .size = buffer_resource.m_data_size,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    });

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

    if (const auto result = vmaCreateBuffer(m_device.allocator(), &buffer_ci, &alloc_ci, &physical.m_buffer,
                                            &physical.m_allocation, &physical.m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: Failed to create buffer!", result, "rendergraph buffer");
    }

    // TODO: Use a better naming system for memory resources inside of rendergraph
    vmaSetAllocationName(m_device.allocator(), physical.m_allocation, "rendergraph buffer");
}

void RenderGraph::build_image(const TextureResource &texture_resource, PhysicalImage &physical,
                              VmaAllocationCreateInfo *alloc_ci) const {
    const auto image_ci = wrapper::make_info<VkImageCreateInfo>({
        .imageType = VK_IMAGE_TYPE_2D,
        .format = texture_resource.m_format,
        .extent{
            // TODO: Support textures with dimensions not equal to back buffer size.
            .width = m_swapchain.extent().width,
            .height = m_swapchain.extent().height,
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = texture_resource.m_usage == TextureUsage::DEPTH_STENCIL_BUFFER
                     ? static_cast<VkImageUsageFlags>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                     : static_cast<VkImageUsageFlags>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    });

    VmaAllocationInfo alloc_info;
    // TODO: Assign proper name to this image inside of rendergraph
    if (const auto result = vmaCreateImage(m_device.allocator(), &image_ci, alloc_ci, &physical.m_image,
                                           &physical.m_allocation, &alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImage failed!", result);
    }

    // TODO: Use a better naming system for memory resources inside of rendergraph
    vmaSetAllocationName(m_device.allocator(), physical.m_allocation, "rendergraph image");
}

void RenderGraph::build_image_view(const TextureResource &texture_resource, PhysicalImage &physical) const {
    const auto image_view_ci = wrapper::make_info<VkImageViewCreateInfo>({
        .image = physical.m_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = texture_resource.m_format,
        .subresourceRange{
            .aspectMask = static_cast<VkImageAspectFlags>(texture_resource.m_usage == TextureUsage::DEPTH_STENCIL_BUFFER
                                                              ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
                                                              : VK_IMAGE_ASPECT_COLOR_BIT),
            .levelCount = 1,
            .layerCount = 1,
        },
    });

    if (const auto result = vkCreateImageView(m_device.device(), &image_view_ci, nullptr, &physical.m_image_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed!", result, texture_resource.m_name);
    }
}

void RenderGraph::record_command_buffer(const RenderStage *stage, const wrapper::commands::CommandBuffer &cmd_buf,
                                        const std::uint32_t image_index) const {
    const PhysicalStage &physical = *stage->m_physical;

    // Record render pass for graphics stages.
    const auto *graphics_stage = stage->as<GraphicsStage>();
    if (graphics_stage != nullptr) {
        const auto *phys_graphics_stage = physical.as<PhysicalGraphicsStage>();
        assert(phys_graphics_stage != nullptr);

        std::array<VkClearValue, 2> clear_values{};
        if (graphics_stage->m_clears_screen) {
            clear_values[0].color = {0, 0, 0, 0};
            clear_values[1].depthStencil = {1.0f, 0};
        }

        cmd_buf.begin_render_pass(wrapper::make_info<VkRenderPassBeginInfo>({
            .renderPass = phys_graphics_stage->m_render_pass,
            .framebuffer = phys_graphics_stage->m_framebuffers.at(image_index).framebuffer(),
            .renderArea{
                .extent = m_swapchain.extent(),
            },
            .clearValueCount = static_cast<std::uint32_t>(clear_values.size()),
            .pClearValues = clear_values.data(),
        }));
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

    cmd_buf.bind_pipeline(physical.m_pipeline->pipeline());
    stage->m_on_record(physical, cmd_buf);

    if (graphics_stage != nullptr) {
        cmd_buf.end_render_pass();
    }

    // TODO: Find a more performant solution instead of placing a full memory barrier after each stage!
    cmd_buf.full_barrier();
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

    // Build a simple subpass that just waits for the output colour vector to be written by the fragment shader. In the
    // future, we may want to make use of subpasses more.
    const VkSubpassDependency subpass_dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    const VkSubpassDescription subpass_description{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = static_cast<std::uint32_t>(colour_refs.size()),
        .pColorAttachments = colour_refs.data(),
        .pDepthStencilAttachment = !depth_refs.empty() ? depth_refs.data() : nullptr,
    };

    const auto render_pass_ci = wrapper::make_info<VkRenderPassCreateInfo>({
        .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass_description,
        .dependencyCount = 1,
        .pDependencies = &subpass_dependency,
    });

    if (const auto result = vkCreateRenderPass(m_device.device(), &render_pass_ci, nullptr, &physical.m_render_pass);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateRenderPass failed!", result, stage->name());
    }
    m_device.set_debug_name(physical.m_render_pass, stage->name());
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

        vertex_bindings.push_back({
            .binding = binding,
            .stride = static_cast<std::uint32_t>(buffer_resource->m_element_size),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        });
    }

    auto blend_attachment = stage->m_blend_attachment;
    blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    physical.m_pipeline = m_graphics_pipeline_builder->set_descriptor_set_layouts(stage->m_descriptor_layouts)
                              .set_push_constant_ranges(stage->m_push_constant_ranges)
                              .set_vertex_input_bindings(vertex_bindings)
                              .set_vertex_input_attributes(attribute_bindings)
                              .set_input_assembly(wrapper::make_info<VkPipelineInputAssemblyStateCreateInfo>({
                                  .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                                  .primitiveRestartEnable = VK_FALSE,
                              }))
                              .set_depth_stencil(wrapper::make_info<VkPipelineDepthStencilStateCreateInfo>({
                                  .depthTestEnable = stage->m_depth_test ? VK_TRUE : VK_FALSE,
                                  .depthWriteEnable = stage->m_depth_write ? VK_TRUE : VK_FALSE,
                                  .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
                              }))
                              .set_rasterization(wrapper::make_info<VkPipelineRasterizationStateCreateInfo>({
                                  .polygonMode = VK_POLYGON_MODE_FILL,
                                  .cullMode = VK_CULL_MODE_BACK_BIT,
                                  .frontFace = VK_FRONT_FACE_CLOCKWISE,
                                  .lineWidth = 1.0f,
                              }))
                              .set_multisampling(VK_SAMPLE_COUNT_1_BIT, 1.0f)
                              .add_color_blend_attachment(blend_attachment)
                              .set_scissor({
                                  .extent = m_swapchain.extent(),
                              })
                              .set_render_pass(physical.m_render_pass)
                              .set_shaders(stage->m_shaders)
                              .set_viewport({
                                  .width = static_cast<float>(m_swapchain.extent().width),
                                  .height = static_cast<float>(m_swapchain.extent().height),
                                  .maxDepth = 1.0f,
                              })
                              .build(stage->name(), false);
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

    m_log->trace("Final stage order:");
    for (auto *stage : m_stage_stack) {
        m_log->trace("  - {}", stage->m_name);
    }

    // Create physical resources. For now, each buffer or texture resource maps directly to either a VkBuffer or VkImage
    // respectively. Every physical resource also has a VmaAllocation.
    // TODO: Resource aliasing (i.e. reusing the same physical resource for multiple resources).
    m_log->trace("Allocating physical resource for buffers:");

    for (auto &buffer_resource : m_buffer_resources) {
        m_log->trace("   - {}", buffer_resource->m_name);
        buffer_resource->m_physical = std::make_shared<PhysicalBuffer>(m_device);
    }

    m_log->trace("Allocating physical resource for texture:");

    for (auto &texture_resource : m_texture_resources) {
        m_log->trace("   - {}", texture_resource->m_name);
        // Back buffer gets special handling.
        if (texture_resource->m_usage == TextureUsage::BACK_BUFFER) {
            // TODO: Move image views from wrapper::Swapchain to PhysicalBackBuffer.
            texture_resource->m_physical = std::make_shared<PhysicalBackBuffer>(m_device, m_swapchain);
            continue;
        }

        // TODO: Use a constexpr bool.
        VmaAllocationCreateInfo alloc_ci{};
        alloc_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        auto physical = std::make_shared<PhysicalImage>(m_device);
        texture_resource->m_physical = physical;
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
                image_views.reserve(back_buffers.size() + images.size());
                for (auto *const img_view : m_swapchain.image_views()) {
                    std::fill_n(std::back_inserter(image_views), back_buffers.size(), img_view);
                    for (const auto *image : images) {
                        image_views.push_back(image->m_image_view);
                    }
                    physical.m_framebuffers.emplace_back(m_device, physical.m_render_pass, image_views, m_swapchain,
                                                         "Framebuffer");
                    image_views.clear();
                }
            }
        }
    }
}

void RenderGraph::render(const std::uint32_t image_index, const wrapper::commands::CommandBuffer &cmd_buf) {
    // Update dynamic buffers.
    for (auto &buffer_resource : m_buffer_resources) {
        if (buffer_resource->m_data_upload_needed) {
            auto &physical = *buffer_resource->m_physical->as<PhysicalBuffer>();

            if (physical.m_buffer != nullptr) {
                vmaDestroyBuffer(m_device.allocator(), physical.m_buffer, physical.m_allocation);
            }

            build_buffer(*buffer_resource, physical);

            // Upload new data.
            assert(physical.m_alloc_info.pMappedData != nullptr);
            std::memcpy(physical.m_alloc_info.pMappedData, buffer_resource->m_data, buffer_resource->m_data_size);
            buffer_resource->m_data_upload_needed = false;
        }
    }

    for (const auto &stage : m_stage_stack) {
        record_command_buffer(stage, cmd_buf, image_index);
    }
}

} // namespace inexor::vulkan_renderer
