#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/render_module.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/per_frame_descriptor_sets.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/queries/query_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/synchronization/pipeline_barrier_batch_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/synchronization/semaphore.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using tools::make_info;
using wrapper::commands::CommandBufferBuilder;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::DescriptorType;
using wrapper::descriptors::PerFrameDescriptorSets;
using wrapper::descriptors::WriteDescriptorSetBuilder;
using wrapper::synchronization::Semaphore;

RenderGraph::RenderGraph(Device &device, const bool use_secondary_command_buffers, const bool one_command_buffer,
                         const std::uint32_t frames_in_flight)
    : m_device(device), m_resource_descriptors(device), m_graphics_pipeline_builder(device),
      m_swapchain_manager(device), m_command_buffer_cache(device, use_secondary_command_buffers),
      m_one_command_buffer(one_command_buffer),
      m_query_pool(std::make_unique<wrapper::queries::QueryPool>(device, 2u * frames_in_flight)),
      m_upload_finished(std::make_unique<Semaphore>(device, "render_graph_upload_finished")),
      m_frame_sync_manager(device), m_staging_buffer(device, "render_graph_upload_arena") {
    VkPhysicalDeviceProperties physical_device_properties{};
    vkGetPhysicalDeviceProperties(m_device.physical_device(), &physical_device_properties);
    m_timestamp_period = physical_device_properties.limits.timestampPeriod;
}

RenderGraph::~RenderGraph() {
    try {
        m_device.wait_idle();
        m_frame_sync_manager.process_deferred_releases(true);
        // All queued GPU work has completed, so emit the final accumulated report before
        // the swapchain and frame-synchronization state is cleared below.
        log_gpu_frame_time();
        log_performance_stats();
        for (auto &release : m_inline_update_pending_releases) {
            release();
        }
        m_inline_update_pending_releases.clear();
        m_inline_update_commands = {};
        m_device.log_vma_statistics();
        m_graphics_passes.clear();
        m_buffers.clear();
        m_textures.clear();
        m_resource_descriptors.clear();
        m_graphics_pipeline_create_functions.clear();
        m_swapchain_manager.clear();
        m_pending_queue_ownership_acquire_barriers.reset();
        m_staging_buffer.reset();
    } catch (...) {}
}

void RenderGraph::synchronize_frame_context() {
    // Get the frame slot count and the current frame slot index from the swapchain manager
    m_frame_slot_count = m_swapchain_manager.frame_slot_count();
    m_current_frame_slot = m_swapchain_manager.current_frame_slot();

    m_staging_buffer.set_frame_context(m_frame_slot_count, m_current_frame_slot);
    m_frame_sync_manager.set_frame_context(m_frame_slot_count, m_current_frame_slot);
    m_resource_descriptors.set_frame_context(m_frame_slot_count, m_current_frame_slot);
    m_command_buffer_cache.set_frame_context(m_frame_slot_count, m_current_frame_slot,
                                             m_frame_sync_manager.frame_slot_submission_fences());

    for (const auto &buffer : m_buffers) {
        buffer->set_frame_context(m_frame_slot_count, m_current_frame_slot);
    }
    for (const auto &texture : m_textures) {
        texture->set_frame_context(m_frame_slot_count, m_current_frame_slot);
    }
}

void RenderGraph::register_frame_submission(const VkFence submission_fence) {
    if (submission_fence == VK_NULL_HANDLE) {
        throw tools::InexorException("Error: Graphics submission returned an invalid fence!");
    }

    // The same submission fence protects both kinds of frame-local ownership:
    // frame-slot resources and the swapchain image acquired for this frame.
    m_frame_sync_manager.mark_frame_slot_submission_fence(submission_fence);
    m_swapchain_manager.mark_frame_swapchains_in_flight(submission_fence);
}

VkFence
RenderGraph::submit_graphics_frame(const std::span<const wrapper::commands::CommandBuffer *const> command_buffers,
                                   const std::span<const wrapper::core::QueueSemaphoreWait> wait_semaphores,
                                   const std::span<const VkSemaphore> signal_semaphores) const {
    if (command_buffers.empty()) {
        throw tools::InexorException("Error: Graphics submission contains no command buffers!");
    }
    return m_device.submit_graphics_command_buffers(command_buffers, wait_semaphores, signal_semaphores);
}

VkFence RenderGraph::submit_graphics_frame(const std::function<void(wrapper::commands::CommandBufferBuilder &)> &record,
                                           const std::span<const wrapper::core::QueueSemaphoreWait> wait_semaphores,
                                           const std::span<const VkSemaphore> signal_semaphores) const {
    return m_device.execute(VK_QUEUE_GRAPHICS_BIT, DebugLabelColor::CYAN, record, wait_semaphores, signal_semaphores);
}

std::weak_ptr<PerFrameDescriptorSets>
RenderGraph::add_resource_descriptor(const std::variant<std::weak_ptr<Buffer>, std::weak_ptr<Texture>> resource,
                                     const VkShaderStageFlags stage, const std::uint32_t dst_binding) {
    // Since resource descriptors have changed, we also need to invalidate the graphics pass secondary command buffers
    // @TODO Only invalidate the graphics pass secondary command buffers that use this resource descriptor
    // @TODO Strictly speaking, this function can only be called before the render graph is built, otherwise we would
    // need to rebuild the render graph (it could be called only after reset_graph() has been called)
    // We therefore should store the rendergraph state in an enum
    invalidate_graphics_pass_secondary_cmd_buffers();
    return std::visit(
        [&](const auto &weak_resource) {
            // Get the resource type (Buffer or Texture) and add it to the resource descriptors
            using Resource = typename std::decay_t<decltype(weak_resource)>::element_type;
            const auto shared_resource = weak_resource.lock();
            if (!shared_resource) {
                throw tools::InexorException("Invalid resource!");
            }
            // @TODO Add support for other descriptor types
            if constexpr (std::same_as<Resource, Buffer>) {
                if (shared_resource->type() != BufferType::UNIFORM_BUFFER) {
                    throw tools::InexorException("Automatic buffer descriptors only support uniform buffers!");
                }
            }
            const auto name = shared_resource->name();
            // @TODO Expose the underlying builder patterns so that the user can customize the descriptor set layout and
            // write descriptor set creation, e.g. add multiple bindings, arrayed descriptors, etc.
            return m_resource_descriptors.add_resource_descriptor(
                name,
                [=](DescriptorSetLayoutBuilder &builder) {
                    // Add the descriptor to the layout builder
                    if constexpr (std::same_as<Resource, Buffer>) {
                        return builder.add(DescriptorType::UNIFORM_BUFFER, stage).build(name);
                    } else {
                        return builder.add(DescriptorType::COMBINED_IMAGE_SAMPLER, stage).build(name);
                    }
                },
                [=](WriteDescriptorSetBuilder &builder, VkDescriptorSet descriptor_set) {
                    // Add the descriptor to the write descriptor set builder
                    return builder.add(descriptor_set, weak_resource, dst_binding).build();
                });
        },
        resource);
}

std::weak_ptr<Buffer> RenderGraph::add_buffer(std::string name, const BufferType type, std::function<void()> on_update,
                                              const BufferUpdateMode update_mode) {
    // Create a shared pointer for the new buffer inside of rendergraph and return a weak pointer to external code
    // This memory ownership models dictates that buffer are owned by rendergraph and not by external code!
    return m_buffers.emplace_back(
        std::make_shared<Buffer>(m_device, std::move(name), type, std::move(on_update), update_mode));
}

std::weak_ptr<GraphicsPass> RenderGraph::add_graphics_pass(OnBuildGraphicsPass on_build_graphics_pass) {
    // Invoke the graphics pipeline create lambda, insert the shared pointer into vector, and return weak pointer
    // This memory ownership models dictates that graphics passes are owned by rendergraph and not by external code!
    m_swapchain_manager.mark_swapchain_cache_dirty();
    m_recording_tasks_dirty = true;
    return m_graphics_passes.emplace_back(std::move(on_build_graphics_pass(m_graphics_pass_builder)));
}

void RenderGraph::add_graphics_pipeline(OnBuildGraphicsPipeline on_build_graphics_pipeline) {
    // Store the graphics pipeline create function so it can be invoked later when pipeline layout is known,
    // for which we need to know the descriptor set layouts first. Take a moment to realize this ordering requirement.
    m_graphics_pipeline_create_functions.emplace_back(std::move(on_build_graphics_pipeline));
}

void RenderGraph::register_render_module(RenderModule &render_module) {
    m_render_modules.emplace_back(&render_module);
}

void RenderGraph::unregister_render_module(RenderModule &render_module) {
    const auto it = std::remove(m_render_modules.begin(), m_render_modules.end(), &render_module);
    m_render_modules.erase(it, m_render_modules.end());
}

std::weak_ptr<Texture> RenderGraph::add_texture(std::string name, const TextureUsage usage, const VkFormat format,
                                                const std::uint32_t width, const std::uint32_t height,
                                                const std::uint32_t channels, const VkSampleCountFlagBits sample_count,
                                                std::optional<std::function<void()>> on_update) {
    // Create a shared pointer for the new texture inside of rendergraph and return a weak pointer to external code
    // This memory ownership models dictates that textures are owned by rendergraph and not by external code!
    return m_textures.emplace_back(std::make_shared<Texture>(m_device, std::move(name), usage, format, width, height,
                                                             channels, sample_count, std::move(on_update)));
}

void RenderGraph::invalidate_graphics_pass_secondary_cmd_buffers() {
    m_command_buffer_cache.invalidate_all_secondary_command_buffers();
}

void RenderGraph::invalidate_graphics_passes_using_texture(const Texture &texture) {
    // Iterate through all graphics passes which use this texture and mark the
    // rendering info as dirty so that the graphics pass will be rebuilt next frame
    for (auto *pass : texture.m_graphics_passes_using_texture) {
        pass->m_rendering_info_dirty = true;
    }
}

void RenderGraph::create_graphics_pipelines() {
    m_resource_descriptors.create_descriptor_set_layouts();
    spdlog::trace("Creating {} graphics pipelines", m_graphics_pipeline_create_functions.size());
    // @TODO Mark graphics pipeline builder as static thread_local and create graphics pipelines in parallel
    m_graphics_pipelines.clear();
    m_graphics_pipelines.reserve(m_graphics_pipeline_create_functions.size());
    for (const auto &create_func : m_graphics_pipeline_create_functions) {
        m_graphics_pipelines.emplace_back(std::invoke(create_func, m_graphics_pipeline_builder));
    }
    m_resource_descriptors.mark_descriptor_sets_dirty();
}

void RenderGraph::log_render_graph_overview() const {
    spdlog::trace("RenderGraph overview: {} module(s)", m_render_modules.size());
    for (const auto *module : m_render_modules) {
        if (!module) {
            continue;
        }
        spdlog::trace("  Module: {}", module->m_name);
        spdlog::trace("    Graphics passes: {}", module->m_graphics_passes.size());
        for (const auto &pass_weak : module->m_graphics_passes) {
            const auto pass = pass_weak.lock();
            if (!pass) {
                spdlog::trace("      <expired pass>");
                continue;
            }
            spdlog::trace("      Pass: {}", pass->m_name);
            const auto print_resources = [](const char *label, const auto &resources) {
                std::string names;
                for (const auto &resource : resources) {
                    const auto locked = resource.lock();
                    names += locked ? std::string(locked->name()) : std::string("<expired>");
                    names += ", ";
                }
                if (!names.empty()) {
                    names.erase(names.size() - 2);
                }
                spdlog::trace("        {}: {}", label, names.empty() ? std::string{"<none>"} : names);
            };
            print_resources("Reads buffers", pass->m_buffer_reads);
            {
                std::vector<std::weak_ptr<Texture>> written_textures;
                written_textures.reserve(pass->m_texture_writes.size());
                for (const auto &write_texture : pass->m_texture_writes) {
                    written_textures.emplace_back(write_texture.first);
                }
                print_resources("Writes textures", written_textures);
            }
            {
                std::vector<std::weak_ptr<wrapper::swapchains::Swapchain>> written_swapchains;
                written_swapchains.reserve(pass->m_swapchain_writes.size());
                for (const auto &write_swapchain : pass->m_swapchain_writes) {
                    written_swapchains.emplace_back(write_swapchain.first);
                }
                print_resources("Writes swapchains", written_swapchains);
            }
        }
        spdlog::trace("    Graphics pipelines: {}", module->m_graphics_pipelines.size());
        for (const auto &pipeline_weak : module->m_graphics_pipelines) {
            const auto pipeline = pipeline_weak.lock();
            if (!pipeline) {
                spdlog::trace("      <expired pipeline>");
                continue;
            }
            spdlog::trace("      Pipeline: {}", pipeline->name());
        }
    }
}

void RenderGraph::check_for_cycles() {
    // @TODO Implement!
}

void RenderGraph::compile() {
    check_for_cycles();
    sort_graphics_passes_by_order();
    rebuild_recording_tasks();
    synchronize_frame_context();
    build_texture_graphics_pass_dependencies();
    create_graphics_pipelines();
    log_render_graph_overview();
    invalidate_graphics_pass_secondary_cmd_buffers();
}

void RenderGraph::rebuild_recording_tasks() {
    m_recording_tasks.clear();
    for (std::size_t i = 0; i < m_graphics_passes.size(); ++i) {
        m_recording_tasks.emplace([this, i] {
            const auto &pass = *m_graphics_passes[i];
            if (!m_command_buffer_cache.uses_secondary_command_buffers()) {
                const auto command_buffer_name = "render_graph_pass[" + std::to_string(i) + "]" + m_frame_slot_suffix;
                const auto &command_buffer =
                    m_device.request_named_command_buffer(VK_QUEUE_GRAPHICS_BIT, command_buffer_name);
                CommandBufferBuilder builder(command_buffer);
                builder.begin_debug_label_region(pass.m_name, pass.m_debug_label_color)
                    .begin_rendering(pass.m_rendering_info)
                    .invoke(pass.m_on_record_cmd_buffer)
                    .end_rendering()
                    .end_debug_label_region()
                    .end_command_buffer();
                m_scratch_primary_command_buffers[i] = &command_buffer;
                return;
            }

            const auto inheritance_rendering_info = make_info<VkCommandBufferInheritanceRenderingInfo>({
                .colorAttachmentCount = static_cast<std::uint32_t>(pass.m_cached_color_attachment_formats.size()),
                .pColorAttachmentFormats = pass.m_cached_color_attachment_formats.empty()
                                               ? nullptr
                                               : pass.m_cached_color_attachment_formats.data(),
                .depthAttachmentFormat = pass.m_cached_depth_attachment_format,
                .stencilAttachmentFormat = pass.m_cached_stencil_attachment_format,
                .rasterizationSamples = pass.m_cached_sample_count,
            });
            const auto inheritance_info = make_info<VkCommandBufferInheritanceInfo>({
                .pNext = &inheritance_rendering_info,
            });
            m_scratch_secondary_command_buffers[i] = m_command_buffer_cache.record_secondary_command_buffer(
                pass.m_name, pass.m_cached_render_extent, inheritance_info, pass.m_on_record_cmd_buffer,
                pass.m_cache_secondary_command_buffer);
        });
    }
    m_recording_tasks_dirty = false;
}

void RenderGraph::build_texture_graphics_pass_dependencies() {
    // Clear all previous graphics pass dependencies for all textures
    for (const auto &texture : m_textures) {
        texture->m_graphics_passes_using_texture.clear();
    }
    // Build new graphics pass dependencies for all textures
    for (const auto &pass : m_graphics_passes) {
        for (const auto &write_attachment : pass->m_texture_writes) {
            const auto attachment = write_attachment.first.lock();
            if (attachment) {
                // At runtime, we can simply look up the graphics passes that write to a texture and then find the
                // graphics passes that read from that texture
                attachment->m_graphics_passes_using_texture.insert(pass.get());
            }
        }
    }
}

void RenderGraph::rebuild_graphics_pass_texture_rendering_info(GraphicsPass &pass) {
    if (!pass.m_rendering_info_dirty) {
        return;
    }
    auto clear_values_equal = [](const std::optional<VkClearValue> &lhs, const std::optional<VkClearValue> &rhs) {
        if (lhs.has_value() != rhs.has_value()) {
            return false;
        }
        if (!lhs.has_value()) {
            return true;
        }
        return std::memcmp(&lhs.value(), &rhs.value(), sizeof(VkClearValue)) == 0;
    };

    auto extents_equal = [](const VkExtent2D &lhs, const VkExtent2D &rhs) {
        return lhs.width == rhs.width && lhs.height == rhs.height;
    };

    auto get_image_layout = [](const TextureUsage usage) {
        switch (usage) {
        case TextureUsage::COLOR_ATTACHMENT:
        case TextureUsage::DEFAULT:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case TextureUsage::DEPTH_ATTACHMENT:
        case TextureUsage::STENCIL_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        default:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    };

    auto make_rendering_attachment_info = [&](const VkImageView image_view, const VkImageLayout image_layout,
                                              const std::optional<VkClearValue> &clear_value,
                                              const VkImageView resolve_image_view = VK_NULL_HANDLE,
                                              const bool enable_resolve = false) {
        const bool has_resolve = enable_resolve && resolve_image_view != VK_NULL_HANDLE;
        return make_info<VkRenderingAttachmentInfo>({
            .imageView = image_view,
            .imageLayout = image_layout,
            .resolveMode = has_resolve ? VK_RESOLVE_MODE_AVERAGE_BIT : VK_RESOLVE_MODE_NONE,
            .resolveImageView = resolve_image_view,
            .resolveImageLayout = has_resolve ? image_layout : VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clear_value.value_or(VkClearValue{}),
        });
    };

    auto track_attachment_usage = [](const TextureUsage usage, std::size_t &color_attachment_count,
                                     bool &has_depth_attachment, bool &has_stencil_attachment) {
        switch (usage) {
        case TextureUsage::COLOR_ATTACHMENT:
            ++color_attachment_count;
            break;
        case TextureUsage::DEPTH_ATTACHMENT:
            has_depth_attachment = true;
            break;
        case TextureUsage::STENCIL_ATTACHMENT:
            has_stencil_attachment = true;
            break;
        default:
            break;
        }
    };

    VkExtent2D render_extent{};
    bool render_extent_initialized = false;
    auto shrink_render_extent = [&](const VkExtent2D attachment_extent) {
        if (!render_extent_initialized) {
            render_extent = attachment_extent;
            render_extent_initialized = true;
            return;
        }
        render_extent.width = std::min(render_extent.width, attachment_extent.width);
        render_extent.height = std::min(render_extent.height, attachment_extent.height);
    };

    // NOTE: These are reused per-pass scratch vectors (cleared and refilled every call) instead of fresh locals,
    // to avoid a heap allocation every frame for every pass.
    auto &current_texture_states = pass.m_scratch_current_texture_states;
    current_texture_states.clear();
    current_texture_states.reserve(pass.m_texture_writes.size());

    auto &cached_texture_color_attachment_formats = pass.m_cached_texture_color_attachment_formats;
    cached_texture_color_attachment_formats.clear();
    cached_texture_color_attachment_formats.reserve(pass.m_texture_writes.size());
    pass.m_cached_depth_attachment_format = VK_FORMAT_UNDEFINED;
    pass.m_cached_stencil_attachment_format = VK_FORMAT_UNDEFINED;
    pass.m_cached_sample_count = VK_SAMPLE_COUNT_1_BIT;

    std::size_t color_texture_attachment_count = 0;
    bool has_depth_attachment = false;
    bool has_stencil_attachment = false;

    for (const auto &write_attachment : pass.m_texture_writes) {
        const auto attachment = write_attachment.first.lock();
        if (!attachment) {
            throw std::runtime_error("Error: Graphics pass texture attachment expired!");
        }

        const auto sample_count = attachment->samples();
        const bool is_msaa = sample_count > VK_SAMPLE_COUNT_1_BIT;

        // Check if this MSAA color attachment should resolve to swapchain instead of internal image
        VkImageView resolve_target = VK_NULL_HANDLE;
        if (is_msaa && attachment->usage() == TextureUsage::COLOR_ATTACHMENT && !pass.m_swapchain_writes.empty()) {
            // Will be filled in later with actual swapchain image view
            resolve_target = VK_NULL_HANDLE;
        }

        current_texture_states.push_back({
            .image_view = is_msaa ? attachment->msaa_image_view() : attachment->image_view(),
            .resolve_image_view = resolve_target,
            .image_layout = get_image_layout(attachment->usage()),
            .extent = attachment->extent(),
            .clear_value = write_attachment.second,
            .usage = attachment->usage(),
            .samples = sample_count,
        });
        track_attachment_usage(attachment->usage(), color_texture_attachment_count, has_depth_attachment,
                               has_stencil_attachment);

        // Capture the sample count from the texture attachments
        pass.m_cached_sample_count = attachment->samples();

        switch (attachment->usage()) {
        case TextureUsage::COLOR_ATTACHMENT:
            pass.m_cached_texture_color_attachment_formats.push_back(attachment->format());
            break;
        case TextureUsage::DEPTH_ATTACHMENT:
            pass.m_cached_depth_attachment_format = attachment->format();
            break;
        case TextureUsage::STENCIL_ATTACHMENT:
            pass.m_cached_stencil_attachment_format = attachment->format();
            break;
        default:
            break;
        }
        shrink_render_extent(attachment->extent());
    }

    if (!pass.m_texture_writes.empty() &&
        (!render_extent_initialized || render_extent.width == 0 || render_extent.height == 0)) {
        throw std::runtime_error("Error: Render pass extent is invalid after attachment resize!");
    }

    auto &texture_states = pass.m_cached_texture_attachment_states;
    texture_states = current_texture_states;
    pass.m_cached_texture_color_attachment_count = color_texture_attachment_count;
    pass.m_cached_texture_render_extent =
        render_extent_initialized ? std::optional<VkExtent2D>{render_extent} : std::nullopt;

    // When there are no swapchain writes, m_cached_color_attachment_formats is solely from textures
    if (pass.m_swapchain_writes.empty()) {
        pass.m_cached_color_attachment_formats = cached_texture_color_attachment_formats;
    }
    pass.reset_rendering_info();
    pass.m_color_attachments.clear();
    pass.m_color_attachments.reserve(color_texture_attachment_count + pass.m_swapchain_writes.size());

    auto build_rendering_attachment_info = [&](const GraphicsPass::CachedAttachmentState &attachment_state) {
        return make_rendering_attachment_info(attachment_state.image_view, attachment_state.image_layout,
                                              attachment_state.clear_value, attachment_state.resolve_image_view,
                                              attachment_state.usage == TextureUsage::COLOR_ATTACHMENT &&
                                                  attachment_state.resolve_image_view != VK_NULL_HANDLE);
    };

    for (const auto &attachment_state : texture_states) {
        const auto rendering_attachment = build_rendering_attachment_info(attachment_state);

        switch (attachment_state.usage) {
        case TextureUsage::COLOR_ATTACHMENT:
            pass.m_color_attachments.push_back(rendering_attachment);
            break;
        case TextureUsage::DEPTH_ATTACHMENT:
            pass.m_depth_attachment = rendering_attachment;
            break;
        case TextureUsage::STENCIL_ATTACHMENT:
            pass.m_stencil_attachment = rendering_attachment;
            break;
        default:
            break;
        }
    }

    pass.m_cached_render_extent = render_extent_initialized ? render_extent : VkExtent2D{0, 0};
    pass.m_rendering_info = make_info<VkRenderingInfo>({
        .renderArea =
            {
                .offset = {0, 0},
                .extent = pass.m_cached_render_extent,
            },
        .layerCount = 1,
        .colorAttachmentCount = static_cast<std::uint32_t>(pass.m_color_attachments.size()),
        .pColorAttachments = pass.m_color_attachments.empty() ? nullptr : pass.m_color_attachments.data(),
        .pDepthAttachment = pass.m_depth_attachment.has_value() ? &pass.m_depth_attachment.value() : nullptr,
        .pStencilAttachment = pass.m_stencil_attachment.has_value() ? &pass.m_stencil_attachment.value() : nullptr,
    });
    pass.m_rendering_info_dirty = false;
}

void RenderGraph::refresh_graphics_pass_swapchain_rendering_info(GraphicsPass &pass) {
    if (pass.m_swapchain_writes.empty()) {
        return;
    }
    auto make_rendering_attachment_info = [](const VkImageView image_view, const VkImageLayout image_layout,
                                             const std::optional<VkClearValue> &clear_value,
                                             const VkImageView resolve_image_view = VK_NULL_HANDLE,
                                             const bool enable_resolve = false) {
        const bool has_resolve = enable_resolve && resolve_image_view != VK_NULL_HANDLE;
        return make_info<VkRenderingAttachmentInfo>({
            .imageView = image_view,
            .imageLayout = image_layout,
            .resolveMode = has_resolve ? VK_RESOLVE_MODE_AVERAGE_BIT : VK_RESOLVE_MODE_NONE,
            .resolveImageView = resolve_image_view,
            .resolveImageLayout = has_resolve ? image_layout : VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            // @TODO Expose clear value as parameter
            .clearValue = clear_value.value_or(VkClearValue{}),
        });
    };

    auto &current_swapchain_states = pass.m_scratch_current_swapchain_states;
    current_swapchain_states.clear();
    current_swapchain_states.reserve(pass.m_swapchain_writes.size());
    VkExtent2D render_extent = pass.m_cached_texture_render_extent.value_or(VkExtent2D{});
    bool render_extent_initialized = pass.m_cached_texture_render_extent.has_value();

    const bool resolve_to_swapchain = pass.m_cached_texture_color_attachment_count > 0 &&
                                      pass.m_cached_sample_count > VK_SAMPLE_COUNT_1_BIT &&
                                      !pass.m_swapchain_writes.empty();

    for (const auto &write_swapchain : pass.m_swapchain_writes) {
        const auto swapchain = write_swapchain.first.lock();
        if (!swapchain) {
            throw std::runtime_error("Error: Graphics pass swapchain attachment expired!");
        }
        const auto extent = swapchain->extent();
        current_swapchain_states.push_back({
            .image_view = swapchain->current_swapchain_image_view(),
            .resolve_image_view = VK_NULL_HANDLE,
            .image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .extent = extent,
            .clear_value = write_swapchain.second,
            .usage = TextureUsage::COLOR_ATTACHMENT,
            .samples = VK_SAMPLE_COUNT_1_BIT,
        });

        if (!render_extent_initialized) {
            render_extent = extent;
            render_extent_initialized = true;
        } else {
            render_extent.width = std::min(render_extent.width, extent.width);
            render_extent.height = std::min(render_extent.height, extent.height);
        }
    }

    if (!render_extent_initialized || render_extent.width == 0 || render_extent.height == 0) {
        throw std::runtime_error("Error: Render pass extent is invalid after attachment resize!");
    }
    pass.m_cached_swapchain_attachment_states = current_swapchain_states;

    // Swapchain images always have 1 sample.
    if (!resolve_to_swapchain) {
        pass.m_cached_sample_count = VK_SAMPLE_COUNT_1_BIT;
    }
    auto &cached_color_attachment_formats = pass.m_cached_color_attachment_formats;
    cached_color_attachment_formats.clear();
    cached_color_attachment_formats.reserve(pass.m_cached_texture_color_attachment_formats.size() +
                                            current_swapchain_states.size());
    cached_color_attachment_formats.insert(cached_color_attachment_formats.end(),
                                           pass.m_cached_texture_color_attachment_formats.begin(),
                                           pass.m_cached_texture_color_attachment_formats.end());

    // Rebuild the active color attachments from the cached texture attachments.
    pass.m_color_attachments.clear();
    pass.m_color_attachments.reserve(pass.m_cached_texture_color_attachment_count +
                                     (resolve_to_swapchain ? 0u : current_swapchain_states.size()));

    for (auto &attachment_state : pass.m_cached_texture_attachment_states) {
        if (attachment_state.usage != TextureUsage::COLOR_ATTACHMENT) {
            continue;
        }
        if (resolve_to_swapchain) {
            const auto &write_swapchain = pass.m_swapchain_writes.front();
            const auto swapchain = write_swapchain.first.lock();
            if (!swapchain) {
                throw std::runtime_error("Error: Graphics pass swapchain attachment expired!");
            }
            attachment_state.resolve_image_view = swapchain->current_swapchain_image_view();
        }
        pass.m_color_attachments.push_back(
            make_rendering_attachment_info(attachment_state.image_view, attachment_state.image_layout,
                                           attachment_state.clear_value, attachment_state.resolve_image_view,
                                           attachment_state.usage == TextureUsage::COLOR_ATTACHMENT &&
                                               attachment_state.resolve_image_view != VK_NULL_HANDLE));
    }
    if (!resolve_to_swapchain) {
        for (const auto &attachment_state : current_swapchain_states) {
            pass.m_color_attachments.push_back(make_rendering_attachment_info(
                attachment_state.image_view, attachment_state.image_layout, attachment_state.clear_value));
        }
    }
    if (!resolve_to_swapchain) {
        for (const auto &write_swapchain : pass.m_swapchain_writes) {
            const auto swapchain = write_swapchain.first.lock();
            if (!swapchain) {
                throw std::runtime_error("Error: Graphics pass swapchain attachment expired!");
            }
            cached_color_attachment_formats.push_back(swapchain->image_format());
        }
    }
    pass.m_cached_render_extent = render_extent;
    pass.m_rendering_info = make_info<VkRenderingInfo>({
        .renderArea =
            {
                .offset = {0, 0},
                .extent = render_extent,
            },
        .layerCount = 1,
        .colorAttachmentCount = static_cast<std::uint32_t>(pass.m_color_attachments.size()),
        .pColorAttachments = pass.m_color_attachments.empty() ? nullptr : pass.m_color_attachments.data(),
        .pDepthAttachment = pass.m_depth_attachment.has_value() ? &pass.m_depth_attachment.value() : nullptr,
        .pStencilAttachment = pass.m_stencil_attachment.has_value() ? &pass.m_stencil_attachment.value() : nullptr,
    });
}

void RenderGraph::prepare_graphics_pass_for_recording(GraphicsPass &pass) {
    if (pass.m_rendering_info_dirty) {
        rebuild_graphics_pass_texture_rendering_info(pass);
    }
    if (!pass.m_swapchain_writes.empty()) {
        refresh_graphics_pass_swapchain_rendering_info(pass);
    }
}

void RenderGraph::record_command_buffer_for_pass(const CommandBuffer &cmd_buf, GraphicsPass &pass) {
    prepare_graphics_pass_for_recording(pass);

    const auto inheritance_rendering_info = make_info<VkCommandBufferInheritanceRenderingInfo>({
        .colorAttachmentCount = static_cast<std::uint32_t>(pass.m_cached_color_attachment_formats.size()),
        .pColorAttachmentFormats =
            pass.m_cached_color_attachment_formats.empty() ? nullptr : pass.m_cached_color_attachment_formats.data(),
        .depthAttachmentFormat = pass.m_cached_depth_attachment_format,
        .stencilAttachmentFormat = pass.m_cached_stencil_attachment_format,
        .rasterizationSamples = pass.m_cached_sample_count,
    });

    const auto inheritance_info = make_info<VkCommandBufferInheritanceInfo>({
        .pNext = &inheritance_rendering_info,
    });

    m_command_buffer_cache.record_secondary_command_buffer(cmd_buf, pass.m_name, pass.m_debug_label_color,
                                                           pass.m_cached_render_extent, inheritance_info,
                                                           pass.m_rendering_info, pass.m_on_record_cmd_buffer);
}

void RenderGraph::render() {
    using Clock = std::chrono::steady_clock;
    // These timings are also used by the explicit performance report (P hotkey),
    // so collect them independently of the current spdlog level.
    constexpr bool collect_cpu_stats = true;
    const auto frame_start = collect_cpu_stats ? Clock::now() : Clock::time_point{};
    m_frame_sync_manager.process_deferred_releases(false);
    const auto housekeeping_end = collect_cpu_stats ? Clock::now() : Clock::time_point{};
    m_swapchain_manager.collect_frame_swapchains(m_graphics_passes);
    m_swapchain_manager.synchronize_frame_context();
    synchronize_frame_context();
    const auto frame_context_end = collect_cpu_stats ? Clock::now() : Clock::time_point{};
    const auto waited_frame_fence = m_frame_sync_manager.wait_for_current_frame_slot();
    if (!m_swapchain_manager.acquire_next_images(waited_frame_fence)) {
        return;
    }
    const auto acquire_end = collect_cpu_stats ? Clock::now() : Clock::time_point{};
    update_resources();
    const auto resources_end = collect_cpu_stats ? Clock::now() : Clock::time_point{};

    auto &render_wait_semaphores = m_scratch_render_wait_semaphores;
    render_wait_semaphores.clear();
    render_wait_semaphores.reserve(m_swapchain_manager.image_available_semaphores().size() +
                                   (m_upload_submission_pending ? 1u : 0u));

    for (const auto semaphore : m_swapchain_manager.image_available_semaphores()) {
        render_wait_semaphores.push_back({
            .semaphore = semaphore,
            // Swapchain images are consumed as color attachments. Waiting at the
            // first stage that accesses the image avoids stalling unrelated work.
            .stage_mask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        });
    }
    if (m_upload_submission_pending) {
        const auto upload_wait_stage_mask = m_upload_wait_stage_mask == VK_PIPELINE_STAGE_2_NONE
                                                ? VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
                                                : m_upload_wait_stage_mask;
        render_wait_semaphores.push_back({
            .semaphore = m_upload_finished->semaphore(),
            .stage_mask = upload_wait_stage_mask,
        });
    }
    if (m_resource_descriptors.descriptor_sets_dirty()) {
        if (m_resource_descriptors.update_write_descriptor_sets()) {
            invalidate_graphics_pass_secondary_cmd_buffers();
        }
    }
    bool cached_recording_invalidated = false;
    for (const auto &pass : m_graphics_passes) {
        if (pass->m_cached_recording_invalidated) {
            cached_recording_invalidated = true;
            pass->m_cached_recording_invalidated = false;
        }
    }
    if (cached_recording_invalidated) {
        invalidate_graphics_pass_secondary_cmd_buffers();
    }

    if (m_recording_tasks_dirty) {
        rebuild_recording_tasks();
    }
    const auto descriptors_end = collect_cpu_stats ? Clock::now() : Clock::time_point{};
    const auto query_base = static_cast<std::uint32_t>(m_current_frame_slot * 2);

    VkFence render_submit_fence = VK_NULL_HANDLE;
    Clock::time_point submit_start{};
    if (m_one_command_buffer) {
        render_submit_fence = submit_graphics_frame(
            [&](CommandBufferBuilder &builder) {
                if (m_query_pool) {
                    builder.reset_query_pool(*m_query_pool, query_base, 2)
                        .write_timestamp(*m_query_pool, query_base, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
                }
                if (m_inline_update_commands) {
                    builder.invoke(m_inline_update_commands);
                }
                m_pending_queue_ownership_acquire_barriers.flush_if_not_empty(builder);
                m_swapchain_manager.prepare_swapchains_for_rendering(builder);

                for (const auto &pass : m_graphics_passes) {
                    prepare_graphics_pass_for_recording(*pass);
                    builder.begin_debug_label_region(pass->m_name, pass->m_debug_label_color)
                        .begin_rendering(pass->m_rendering_info)
                        .invoke(pass->m_on_record_cmd_buffer)
                        .end_rendering()
                        .end_debug_label_region();
                }

                m_swapchain_manager.prepare_swapchains_for_presenting(builder);
                if (m_query_pool) {
                    builder.write_timestamp(*m_query_pool, query_base + 1, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
                }
                if (collect_cpu_stats) {
                    submit_start = Clock::now();
                }
            },
            render_wait_semaphores, m_swapchain_manager.rendering_finished_semaphores());
    } else if (!m_command_buffer_cache.uses_secondary_command_buffers()) {
        m_frame_slot_suffix = "[slot " + std::to_string(m_current_frame_slot) + "]";
        const auto &frame_start_command_buffer = m_device.request_named_command_buffer(
            VK_QUEUE_GRAPHICS_BIT, "render_graph_frame_start" + m_frame_slot_suffix);
        CommandBufferBuilder frame_start_builder(frame_start_command_buffer);
        frame_start_builder.begin_debug_label_region("RenderGraph frame start",
                                                     wrapper::core::get_debug_label_color(DebugLabelColor::CYAN));
        if (m_query_pool) {
            frame_start_builder.reset_query_pool(*m_query_pool, query_base, 2)
                .write_timestamp(*m_query_pool, query_base, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        }
        if (m_inline_update_commands) {
            frame_start_builder.invoke(m_inline_update_commands);
        }
        // Acquire ownership of resources uploaded on a transfer queue before recording any pass that reads them.
        m_pending_queue_ownership_acquire_barriers.flush_if_not_empty(frame_start_builder);
        m_swapchain_manager.prepare_swapchains_for_rendering(frame_start_builder);
        frame_start_builder.end_debug_label_region().end_command_buffer();

        for (const auto &pass : m_graphics_passes) {
            prepare_graphics_pass_for_recording(*pass);
        }

        m_scratch_primary_command_buffers.resize(m_graphics_passes.size());
        m_device.taskflow_executor().run(m_recording_tasks).get();

        const auto &frame_end_command_buffer = m_device.request_named_command_buffer(
            VK_QUEUE_GRAPHICS_BIT, "render_graph_frame_end" + m_frame_slot_suffix);
        CommandBufferBuilder frame_end_builder(frame_end_command_buffer);
        frame_end_builder.begin_debug_label_region("RenderGraph frame end",
                                                   wrapper::core::get_debug_label_color(DebugLabelColor::CYAN));
        m_swapchain_manager.prepare_swapchains_for_presenting(frame_end_builder);
        if (m_query_pool) {
            frame_end_builder.write_timestamp(*m_query_pool, query_base + 1, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
        }
        frame_end_builder.end_debug_label_region().end_command_buffer();

        // Only the final buffer owns a submission fence. Acquiring the frame slot above protects all
        // buffers in this batch from being rerecorded while a previous use is still in flight.
        auto &batch = m_scratch_primary_batch_command_buffers;
        batch.clear();
        batch.reserve(m_scratch_primary_command_buffers.size() + 2);
        batch.push_back(&frame_start_command_buffer);
        batch.insert(batch.end(), m_scratch_primary_command_buffers.begin(), m_scratch_primary_command_buffers.end());
        batch.push_back(&frame_end_command_buffer);
        if (collect_cpu_stats) {
            submit_start = Clock::now();
        }
        render_submit_fence =
            submit_graphics_frame(batch, render_wait_semaphores, m_swapchain_manager.rendering_finished_semaphores());
    } else {
        render_submit_fence = submit_graphics_frame(
            [&](CommandBufferBuilder &builder) {
                if (m_query_pool) {
                    builder.reset_query_pool(*m_query_pool, query_base, 2)
                        .write_timestamp(*m_query_pool, query_base, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
                }
                if (m_inline_update_commands) {
                    builder.invoke(m_inline_update_commands);
                }
                // Acquire ownership of any buffers/images that were uploaded on a transfer queue whose family differs
                // from the graphics queue family, before they are read by any pass below.
                m_pending_queue_ownership_acquire_barriers.flush_if_not_empty(builder);

                m_swapchain_manager.prepare_swapchains_for_rendering(builder);
                for (const auto &pass : m_graphics_passes) {
                    prepare_graphics_pass_for_recording(*pass);
                }

                m_scratch_secondary_command_buffers.resize(m_graphics_passes.size());
                m_device.taskflow_executor().run(m_recording_tasks).get();

                for (std::size_t i = 0; i < m_graphics_passes.size(); ++i) {
                    const auto &pass = *m_graphics_passes[i];
                    m_command_buffer_cache.execute_secondary_command_buffer(
                        builder.command_buffer(), pass.m_name, pass.m_debug_label_color, pass.m_rendering_info,
                        m_scratch_secondary_command_buffers[i]);
                }

                m_swapchain_manager.prepare_swapchains_for_presenting(builder);
                if (m_query_pool) {
                    builder.write_timestamp(*m_query_pool, query_base + 1, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
                }
                if (collect_cpu_stats) {
                    submit_start = Clock::now();
                }
            },
            render_wait_semaphores, m_swapchain_manager.rendering_finished_semaphores());
    }
    const auto submit_end = collect_cpu_stats ? Clock::now() : Clock::time_point{};

    if (m_query_pool) {
        m_query_pool_has_results = true;
    }

    m_upload_submission_pending = false;

    if (!m_inline_update_pending_releases.empty()) {
        std::vector<VkFence> release_wait_fences = m_frame_sync_manager.frame_slot_submission_fences();
        release_wait_fences.push_back(render_submit_fence);
        for (auto &release : m_inline_update_pending_releases) {
            m_frame_sync_manager.defer_release(release_wait_fences, std::move(release));
        }
        m_inline_update_pending_releases.clear();
    }

    m_inline_update_commands = {};
    register_frame_submission(render_submit_fence);
    m_last_gpu_query_frame_slot = m_current_frame_slot;
    ++m_gpu_query_serial;
    const auto finalize_end = collect_cpu_stats ? Clock::now() : Clock::time_point{};
    m_swapchain_manager.present(m_swapchain_manager.rendering_finished_semaphores());
    if (collect_cpu_stats) {
        const auto frame_end = Clock::now();
        const auto elapsed_ns = [](const Clock::time_point start, const Clock::time_point end) {
            return static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
        };
        CpuPhaseDurations durations{};
        durations[static_cast<std::size_t>(CpuPhase::Housekeeping)] = elapsed_ns(frame_start, housekeeping_end);
        durations[static_cast<std::size_t>(CpuPhase::FrameContext)] = elapsed_ns(housekeeping_end, frame_context_end);
        durations[static_cast<std::size_t>(CpuPhase::Acquire)] = elapsed_ns(frame_context_end, acquire_end);
        durations[static_cast<std::size_t>(CpuPhase::Resources)] = elapsed_ns(acquire_end, resources_end);
        durations[static_cast<std::size_t>(CpuPhase::Descriptors)] = elapsed_ns(resources_end, descriptors_end);
        durations[static_cast<std::size_t>(CpuPhase::Recording)] = elapsed_ns(descriptors_end, submit_start);
        durations[static_cast<std::size_t>(CpuPhase::Submit)] = elapsed_ns(submit_start, submit_end);
        durations[static_cast<std::size_t>(CpuPhase::Finalize)] = elapsed_ns(submit_end, finalize_end);
        durations[static_cast<std::size_t>(CpuPhase::Present)] = elapsed_ns(finalize_end, frame_end);
        const auto frame_total_ns = elapsed_ns(frame_start, frame_end);
        ++m_cpu_frame_stats.frames;
        m_cpu_frame_stats.total_ns += frame_total_ns;
        m_cpu_frame_stats.max_frame_ns = std::max(m_cpu_frame_stats.max_frame_ns, frame_total_ns);
        for (std::size_t i = 0; i < cpu_phase_count; ++i) {
            m_cpu_frame_stats.phase_total_ns[i] += durations[i];
            m_cpu_frame_stats.phase_max_ns[i] = std::max(m_cpu_frame_stats.phase_max_ns[i], durations[i]);
        }
    }
}

void RenderGraph::log_gpu_frame_time() const {
    if (!m_query_pool || !m_query_pool_has_results || !m_last_gpu_query_frame_slot) {
        return;
    }
    const auto fence = m_frame_sync_manager.frame_slot_submission_fence(*m_last_gpu_query_frame_slot);
    if (fence == VK_NULL_HANDLE || vkGetFenceStatus(m_device.device(), fence) != VK_SUCCESS) {
        spdlog::trace("GPU frame time is not ready yet");
        return;
    }

    const auto results = m_query_pool->try_get_results(static_cast<std::uint32_t>(*m_last_gpu_query_frame_slot * 2), 2);
    if (!results) {
        return;
    }
    if (results->size() < 4) {
        return;
    }
    const auto &values = *results;
    if (values[1] == 0 || values[3] == 0) {
        return;
    }
    const auto elapsed_ticks = values[2] - values[0];
    const double elapsed_ms =
        static_cast<double>(elapsed_ticks) * static_cast<double>(m_timestamp_period) / 1'000'000.0;
    spdlog::trace("GPU frame time: {:.3f} ms [ticks={}]", elapsed_ms, elapsed_ticks);
}

void RenderGraph::log_performance_stats() const {
    const auto ns_to_ms = [](const std::uint64_t total_ns, const std::uint64_t count) {
        return count == 0 ? 0.0 : static_cast<double>(total_ns) / static_cast<double>(count) / 1'000'000.0;
    };

    spdlog::trace("Performance averages over {} CPU frames ({} frame slots):", m_cpu_frame_stats.frames,
                  m_frame_slot_count);
    if (m_cpu_frame_stats.frames != 0) {
        spdlog::trace("  CPU frame: {:.3f} ms (max {:.3f} ms)",
                      ns_to_ms(m_cpu_frame_stats.total_ns, m_cpu_frame_stats.frames),
                      static_cast<double>(m_cpu_frame_stats.max_frame_ns) / 1'000'000.0);
        constexpr std::array<const char *, cpu_phase_count> phase_names{"housekeeping", "acquire",     "frame-context",
                                                                        "resources",    "descriptors", "recording",
                                                                        "submit",       "finalize",    "present"};
        for (std::size_t i = 0; i < cpu_phase_count; ++i) {
            spdlog::trace("  CPU {:<13}: {:.3f} ms (max {:.3f} ms)", phase_names[i],
                          ns_to_ms(m_cpu_frame_stats.phase_total_ns[i], m_cpu_frame_stats.frames),
                          static_cast<double>(m_cpu_frame_stats.phase_max_ns[i]) / 1'000'000.0);
        }
    }

    const auto &wait_stats = m_frame_sync_manager.wait_stats();
    spdlog::trace("  Frame-slot fence waits: {} calls, avg {:.3f} ms, max {:.3f} ms", wait_stats.waits,
                  ns_to_ms(wait_stats.total_ns, wait_stats.waits),
                  static_cast<double>(wait_stats.max_ns) / 1'000'000.0);

    if (m_gpu_frame_count != 0) {
        spdlog::trace("  GPU frame: {} samples, avg {:.3f} ms, max {:.3f} ms", m_gpu_frame_count,
                      m_gpu_frame_total_ms / static_cast<double>(m_gpu_frame_count), m_gpu_frame_max_ms);
    }

    for (const auto &swapchain : m_swapchain_manager.frame_swapchains()) {
        const auto &stats = swapchain->inflight_stats();
        spdlog::trace("  Swapchain '{}':", swapchain->name());
        spdlog::trace("    acquire avg/max {:.3f}/{:.3f} ms ({})",
                      ns_to_ms(stats.acquire_total_ns, static_cast<std::uint64_t>(stats.acquires)),
                      static_cast<double>(stats.acquire_max_ns) / 1'000'000.0, stats.acquires);
        spdlog::trace("    present avg/max {:.3f}/{:.3f} ms ({})",
                      ns_to_ms(stats.present_total_ns, static_cast<std::uint64_t>(stats.presents)),
                      static_cast<double>(stats.present_max_ns) / 1'000'000.0, stats.presents);
        spdlog::trace("    image-fence wait avg/max {:.3f}/{:.3f} ms ({})",
                      ns_to_ms(stats.image_wait_total_ns, static_cast<std::uint64_t>(stats.image_waits)),
                      static_cast<double>(stats.image_wait_max_ns) / 1'000'000.0, stats.image_waits);
    }
}

std::optional<double> RenderGraph::try_get_gpu_frame_time_ms() const {
    if (!m_query_pool || !m_query_pool_has_results || !m_last_gpu_query_frame_slot) {
        return std::nullopt;
    }

    const auto fence = m_frame_sync_manager.frame_slot_submission_fence(*m_last_gpu_query_frame_slot);
    if (fence == VK_NULL_HANDLE || vkGetFenceStatus(m_device.device(), fence) != VK_SUCCESS) {
        return std::nullopt;
    }

    const auto results = m_query_pool->try_get_results(static_cast<std::uint32_t>(*m_last_gpu_query_frame_slot * 2), 2);
    if (!results || results->size() < 4) {
        return std::nullopt;
    }

    const auto &values = *results;
    const bool start_available = values[1] != 0;
    const bool end_available = values[3] != 0;
    if (!start_available || !end_available) {
        return std::nullopt;
    }

    const auto elapsed_ticks = values[2] - values[0];
    const auto elapsed_ms = static_cast<double>(elapsed_ticks) * static_cast<double>(m_timestamp_period) / 1'000'000.0;
    ++m_gpu_frame_count;
    m_gpu_frame_total_ms += elapsed_ms;
    m_gpu_frame_max_ms = std::max(m_gpu_frame_max_ms, elapsed_ms);
    return elapsed_ms;
}

void RenderGraph::reset_graph() {
    m_frame_sync_manager.process_deferred_releases(true);
    m_cpu_frame_stats = {};
    m_gpu_frame_count = 0;
    m_gpu_frame_total_ms = 0.0;
    m_gpu_frame_max_ms = 0.0;
    m_gpu_query_serial = 0;
    m_gpu_last_sampled_serial = 0;
    m_gpu_last_frame_ms = 0.0;
    m_last_gpu_query_frame_slot.reset();
    m_staging_buffer.reset();
    m_recording_tasks.clear();
    m_recording_tasks_dirty = true;
    m_scratch_primary_command_buffers.clear();
    m_scratch_secondary_command_buffers.clear();
    m_buffers.clear();
    m_textures.clear();
    m_graphics_passes.clear();
    m_resource_descriptors.clear();
    m_graphics_pipeline_create_functions.clear();
    m_graphics_passes.clear();
    m_swapchain_manager.clear();
    m_upload_submission_pending = false;
    m_upload_wait_stage_mask = VK_PIPELINE_STAGE_2_NONE;
    m_inline_update_commands = {};
    m_inline_update_pending_releases.clear();
    m_pending_queue_ownership_acquire_barriers.reset();
    m_query_pool_has_results = false;
    m_frame_sync_manager.clear();
    m_frame_slot_count = 1;
    m_current_frame_slot = 0;
    m_scratch_pending_buffer_copies.clear();
    m_scratch_pending_texture_copies.clear();
    m_scratch_pending_releases.clear();
    m_scratch_color_attachment_formats.clear();
    m_resource_descriptors.mark_descriptor_sets_dirty();
    invalidate_graphics_pass_secondary_cmd_buffers();
}

void RenderGraph::sort_graphics_passes_by_order() {
    // @TODO Implement this in rendergraph3
    // @TODO Implement ordering mechanisms between passes instead of relying on swapchain write order
}

void RenderGraph::update_resources() {
    m_upload_submission_pending = false;
    m_upload_wait_stage_mask = VK_PIPELINE_STAGE_2_NONE;
    m_inline_update_commands = {};
    m_inline_update_pending_releases.clear();

    bool any_buffer_update_required = false;
    bool any_buffer_gpu_update_required = false;
    bool any_buffer_gpu_update_requires_graphics_queue = false;
    std::vector<Buffer *> pending_direct_buffer_updates;
    std::vector<Buffer *> pending_gpu_buffer_updates;
    pending_direct_buffer_updates.reserve(m_buffers.size());
    pending_gpu_buffer_updates.reserve(m_buffers.size());

    for (const auto &buffer : m_buffers) {
        std::invoke(buffer->m_on_check_for_update);
        if (buffer->m_update_requested) {
            any_buffer_update_required = true;
            if (!buffer->can_update_without_command_buffer()) {
                any_buffer_gpu_update_required = true;
                pending_gpu_buffer_updates.push_back(buffer.get());
                if (buffer->update_mode() == BufferUpdateMode::PER_FRAME_DEVICE_LOCAL) {
                    any_buffer_gpu_update_requires_graphics_queue = true;
                }
            } else {
                pending_direct_buffer_updates.push_back(buffer.get());
            }
        }
    }

    bool any_texture_update_required = false;
    // Attachment-type textures (color/depth/stencil) that need their initial layout transition (or, in principle, a
    // CPU-data re-upload) use graphics-pipeline-only pipeline stages (color attachment output, fragment tests) which
    // are not valid on a transfer-only queue family. Whenever this is the case, we must not route this frame's
    // updates through the transfer queue at all (falls back to the same-queue/inline path further below).
    bool any_attachment_texture_layout_prep_required = false;
    std::vector<Texture *> pending_texture_updates;
    pending_texture_updates.reserve(m_textures.size());
    for (const auto &texture : m_textures) {
        if (texture->m_on_update) {
            std::invoke(texture->m_on_update.value());
        }
        if (texture->m_update_requested) {
            any_texture_update_required = true;
            pending_texture_updates.push_back(texture.get());
            if (texture->usage() != TextureUsage::DEFAULT) {
                const bool needs_initial_layout_prep =
                    !texture->current_frame_resources().m_image ||
                    texture->current_frame_resources().m_image->image() == VK_NULL_HANDLE;
                if (needs_initial_layout_prep || texture->m_src_texture_data_size != 0) {
                    any_attachment_texture_layout_prep_required = true;
                }
            }
        }
    }
    if (!any_buffer_update_required && !any_texture_update_required) {
        return;
    }

    auto &pending_releases = m_scratch_pending_releases;
    pending_releases.clear();
    for (auto *buffer : pending_direct_buffer_updates) {
        buffer->update_without_command_buffer();
    }

    for (auto *texture : pending_texture_updates) {
        if (!texture->current_frame_resources().m_image ||
            texture->current_frame_resources().m_image->image() == VK_NULL_HANDLE) {
            m_resource_descriptors.mark_descriptor_sets_dirty();
            invalidate_graphics_passes_using_texture(*texture);
        }
    }
    if (!any_buffer_gpu_update_required && !any_texture_update_required) {
        return;
    }

    std::size_t required_upload_bytes = 0;
    constexpr std::size_t upload_alignment = 16;
    const auto align_up = [](const std::size_t value, const std::size_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    };

    for (const auto *buffer : pending_gpu_buffer_updates) {
        for (std::size_t slot_index = 0; slot_index < buffer->m_slots.size(); ++slot_index) {
            required_upload_bytes += align_up(buffer->m_src_data_size, upload_alignment);
        }
    }
    for (const auto *texture : pending_texture_updates) {
        if (texture->m_src_texture_data_size > 0) {
            for (std::size_t slot_index = 0; slot_index < texture->m_per_frame_texture_resources.size(); ++slot_index) {
                required_upload_bytes += align_up(texture->m_src_texture_data_size, upload_alignment);
            }
        }
    }

    m_staging_buffer.ensure_capacity(required_upload_bytes, pending_releases);

    // Keep uploads on the graphics queue so the frame uses one submit path end-to-end. This avoids the extra
    // transfer-queue submit and the queue-family ownership transfer overhead, at the cost of giving up async upload
    // overlap with rendering.
    const bool use_transfer_queue = false && !any_attachment_texture_layout_prep_required;
    const bool needs_queue_family_ownership_transfer =
        use_transfer_queue && !m_device.transfer_queue_shares_graphics_family();
    const std::uint32_t transfer_family_index =
        needs_queue_family_ownership_transfer ? m_device.transfer_queue_family_index() : VK_QUEUE_FAMILY_IGNORED;
    const std::uint32_t graphics_family_index =
        needs_queue_family_ownership_transfer ? m_device.graphics_queue_family_index() : VK_QUEUE_FAMILY_IGNORED;

    wrapper::synchronization::PipelineBarrierBatchBuilder pre_copy_barriers;
    wrapper::synchronization::PipelineBarrierBatchBuilder post_copy_barriers;
    m_scratch_pending_buffer_copies.clear();
    m_scratch_pending_texture_copies.clear();
    std::size_t upload_offset = 0;

    for (auto *buffer : pending_gpu_buffer_updates) {
        buffer->create(m_scratch_pending_buffer_copies, m_staging_buffer, upload_offset, pending_releases);
        if (buffer->m_descriptor_resource_changed) {
            m_resource_descriptors.mark_descriptor_sets_dirty();
            invalidate_graphics_pass_secondary_cmd_buffers();
        }
    }

    for (auto *texture : pending_texture_updates) {
        bool texture_was_created = false;
        if (!texture->current_frame_resources().m_image ||
            texture->current_frame_resources().m_image->image() == VK_NULL_HANDLE) {
            texture->create_all();
            texture_was_created = true;
        }
        if (texture_was_created) {
            texture->prepare_initial_layout_barriers(pre_copy_barriers);
            invalidate_graphics_passes_using_texture(*texture);
            invalidate_graphics_pass_secondary_cmd_buffers();
        }
        texture->prepare_update_barriers(pre_copy_barriers);
        texture->collect_update_copies(m_staging_buffer, upload_offset, pending_releases,
                                       m_scratch_pending_texture_copies);
    }

    VkPipelineStageFlags2 upload_wait_stage_mask = VK_PIPELINE_STAGE_2_NONE;
    for (const auto &copy_request : m_scratch_pending_buffer_copies) {
        upload_wait_stage_mask |= copy_request.dst_stage_mask;
    }
    for (const auto &copy_request : m_scratch_pending_texture_copies) {
        upload_wait_stage_mask |= copy_request.post_copy_barrier.dstStageMask;
    }

    auto record_update_commands =
        [this, any_buffer_gpu_update_required, any_texture_update_required, needs_queue_family_ownership_transfer,
         transfer_family_index, graphics_family_index, pre_copy_barriers = std::move(pre_copy_barriers),
         post_copy_barriers = std::move(post_copy_barriers)](CommandBufferBuilder &cmd_buf) mutable {
            // Phase 1: Emit all pre-copy transitions at once.
            pre_copy_barriers.flush_if_not_empty(cmd_buf);
            if (any_buffer_gpu_update_required) {
                for (const auto &copy_request : m_scratch_pending_buffer_copies) {
                    cmd_buf.copy_buffer(copy_request.src_buffer, copy_request.dst_buffer, copy_request.region);

                    const auto dst_stage_mask = copy_request.dst_stage_mask | VK_PIPELINE_STAGE_2_COPY_BIT;
                    const auto dst_access_mask = copy_request.dst_access_mask | VK_ACCESS_2_TRANSFER_WRITE_BIT;
                    if (dst_stage_mask == VK_PIPELINE_STAGE_2_NONE || dst_access_mask == VK_ACCESS_2_NONE) {
                        continue;
                    }

                    if (needs_queue_family_ownership_transfer) {
                        post_copy_barriers.add(make_info<VkBufferMemoryBarrier2>({
                            .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                            .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
                            .dstAccessMask = VK_ACCESS_2_NONE,
                            .srcQueueFamilyIndex = transfer_family_index,
                            .dstQueueFamilyIndex = graphics_family_index,
                            .buffer = copy_request.dst_buffer,
                            .offset = copy_request.region.dstOffset,
                            .size = copy_request.region.size,
                        }));
                        m_pending_queue_ownership_acquire_barriers.add(VkBufferMemoryBarrier2({
                            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
                            .srcAccessMask = VK_ACCESS_2_NONE,
                            .dstStageMask = dst_stage_mask,
                            .dstAccessMask = dst_access_mask,
                            .srcQueueFamilyIndex = transfer_family_index,
                            .dstQueueFamilyIndex = graphics_family_index,
                            .buffer = copy_request.dst_buffer,
                            .offset = copy_request.region.dstOffset,
                            .size = copy_request.region.size,
                        }));
                    } else {
                        post_copy_barriers.add(make_info<VkBufferMemoryBarrier2>({
                            .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
                            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                            .dstStageMask = dst_stage_mask,
                            .dstAccessMask = dst_access_mask,
                            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                            .buffer = copy_request.dst_buffer,
                            .offset = copy_request.region.dstOffset,
                            .size = copy_request.region.size,
                        }));
                    }
                }
            }
            if (any_texture_update_required) {
                for (const auto &copy_request : m_scratch_pending_texture_copies) {
                    cmd_buf.copy_buffer_to_image(copy_request.src_buffer, copy_request.dst_image, copy_request.region);
                    if (needs_queue_family_ownership_transfer) {
                        auto release_barrier = copy_request.post_copy_barrier;
                        release_barrier.srcQueueFamilyIndex = transfer_family_index;
                        release_barrier.dstQueueFamilyIndex = graphics_family_index;
                        release_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
                        release_barrier.dstAccessMask = VK_ACCESS_2_NONE;
                        post_copy_barriers.add(release_barrier);

                        auto acquire_barrier = copy_request.post_copy_barrier;
                        acquire_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
                        acquire_barrier.srcAccessMask = VK_ACCESS_2_NONE;
                        acquire_barrier.srcQueueFamilyIndex = transfer_family_index;
                        acquire_barrier.dstQueueFamilyIndex = graphics_family_index;
                        m_pending_queue_ownership_acquire_barriers.add(acquire_barrier);
                    } else {
                        post_copy_barriers.add(copy_request.post_copy_barrier);
                    }
                }
            }

            // Phase 2: Emit all post-copy visibility/layout barriers at once.
            post_copy_barriers.flush_if_not_empty(cmd_buf);
        };

    if (use_transfer_queue) {
        const std::array<VkSemaphore, 1> upload_signal_semaphore = {m_upload_finished->semaphore()};
        const auto update_fence =
            m_device.execute(VK_QUEUE_TRANSFER_BIT, DebugLabelColor::MAGENTA, record_update_commands,
                             std::span<const VkSemaphore>{}, std::span<const VkSemaphore>(upload_signal_semaphore));

        std::vector<VkFence> release_wait_fences = m_frame_sync_manager.frame_slot_submission_fences();
        release_wait_fences.push_back(update_fence);

        m_upload_submission_pending = true;
        m_upload_wait_stage_mask = upload_wait_stage_mask;
        if (!pending_releases.empty()) {
            for (auto &release : pending_releases) {
                m_frame_sync_manager.defer_release(release_wait_fences, std::move(release));
            }
        }
        return;
    }
    // Same-queue path: merge update recording into the main render submission to avoid an extra submit.
    m_inline_update_commands = std::move(record_update_commands);
    m_inline_update_pending_releases = std::move(pending_releases);
}

} // namespace inexor::vulkan_renderer::render_graph
