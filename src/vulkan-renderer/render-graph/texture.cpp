#include "inexor/vulkan-renderer/render-graph/texture.hpp"

#include "inexor/vulkan-renderer/render-graph/staging_buffer.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"
#include "inexor/vulkan-renderer/wrapper/images/image.hpp"
#include "inexor/vulkan-renderer/wrapper/images/sampler.hpp"
#include "inexor/vulkan-renderer/wrapper/synchronization/pipeline_barrier_batch_builder.hpp"

#include <cstring>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Texture::Texture(const wrapper::core::Device &device, std::string name, const TextureUsage usage, const VkFormat format,
                 const std::uint32_t width, const std::uint32_t height, const std::uint32_t channels,
                 const VkSampleCountFlagBits samples, std::optional<std::function<void()>> on_update)
    : m_device(device), m_name(std::move(name)), m_usage(usage), m_on_update(std::move(on_update)), m_format(format),
      m_width(width), m_height(height), m_channels(channels), m_samples(samples) {
    if (m_name.empty()) {
        throw InexorException("Error: Parameter 'name' is an empty string!");
    }
    m_default_sampler = std::make_unique<Sampler>(m_device, "Default Sampler");
}

Texture::Texture(Texture &&other) noexcept : m_device(other.m_device), m_usage(other.m_usage) {
    m_name = std::move(other.m_name);
    m_on_update = std::move(other.m_on_update);
    m_format = other.m_format;
    m_width = other.m_width;
    m_height = other.m_height;
    m_channels = other.m_channels;
    m_samples = other.m_samples;
    m_default_sampler = std::exchange(other.m_default_sampler, nullptr);
    m_update_requested = other.m_update_requested;
    m_src_texture_data = std::exchange(other.m_src_texture_data, nullptr);
    m_src_texture_data_size = other.m_src_texture_data_size;
    m_per_frame_texture_resources = std::move(other.m_per_frame_texture_resources);
    m_frame_slot_count = other.m_frame_slot_count;
    m_current_frame_slot = other.m_current_frame_slot;
}

Texture::~Texture() {
    destroy_all();
}

void Texture::collect_update_copies(StagingBuffer &staging_buffer, std::size_t &upload_offset,
                                    std::vector<std::function<void()>> &pending_releases,
                                    std::vector<PendingTextureCopy> &pending_texture_copies) {
    if (m_src_texture_data_size == 0) {
        m_update_requested = false;
        m_src_texture_data = nullptr;
        m_src_texture_data_size = 0;
        return;
    }

    for (std::size_t slot_index = 0; slot_index < m_per_frame_texture_resources.size(); ++slot_index) {
        auto &slot = m_per_frame_texture_resources[slot_index];
        upload_offset = (upload_offset + 15) & ~std::size_t(15);
        const auto src_offset = static_cast<VkDeviceSize>(upload_offset);
        std::memcpy(static_cast<std::byte *>(staging_buffer.mapped_data()) + upload_offset, m_src_texture_data,
                    m_src_texture_data_size);
        upload_offset += m_src_texture_data_size;

        const auto aspect_mask = [&]() -> VkImageAspectFlags {
            switch (m_usage) {
            case TextureUsage::DEPTH_ATTACHMENT:
                switch (m_format) {
                case VK_FORMAT_D16_UNORM:
                case VK_FORMAT_D32_SFLOAT:
                    return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT);
                case VK_FORMAT_S8_UINT:
                    return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_STENCIL_BIT);
                case VK_FORMAT_D16_UNORM_S8_UINT:
                case VK_FORMAT_D24_UNORM_S8_UINT:
                case VK_FORMAT_D32_SFLOAT_S8_UINT:
                    return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
                default:
                    return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT);
                }
            case TextureUsage::STENCIL_ATTACHMENT:
                return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_STENCIL_BIT);
            default:
                return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_COLOR_BIT);
            }
        }();

        pending_texture_copies.push_back({
            .src_buffer = staging_buffer.buffer(),
            .dst_image = slot.m_image->image(),
            .region =
                {
                    .bufferOffset = src_offset,
                    .bufferRowLength = 0,
                    .bufferImageHeight = 0,
                    .imageSubresource =
                        {
                            .aspectMask = aspect_mask,
                            .mipLevel = 0,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                        },
                    .imageOffset = {0, 0, 0},
                    .imageExtent = {m_width, m_height, 1},
                },
            .post_copy_barrier =
                {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                    .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
                    .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                    .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = slot.m_image->m_img,
                    .subresourceRange =
                        {
                            .aspectMask = aspect_mask,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                        },
                },
        });

        slot.m_descriptor_img_info = {
            .sampler = m_default_sampler->sampler(),
            .imageView = slot.m_image->m_img_view,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
    }

    m_update_requested = false;
    m_src_texture_data = nullptr;
    m_src_texture_data_size = 0;
}

void Texture::create_all() {
    for (std::size_t frame_index = 0; frame_index < m_per_frame_texture_resources.size(); ++frame_index) {
        create_per_frame_resources(m_per_frame_texture_resources[frame_index], frame_index);
    }
}

void Texture::create_per_frame_resources(PerFrameTextureResources &frame_resource, const std::size_t slot_index) {
    const auto slot_name =
        m_per_frame_texture_resources.size() > 1 ? m_name + "[slot " + std::to_string(slot_index) + "]" : m_name;
    if (!frame_resource.m_image) {
        frame_resource.m_image = std::make_shared<Image>(m_device, slot_name);
    }
    if (m_samples > VK_SAMPLE_COUNT_1_BIT && !frame_resource.m_msaa_image) {
        frame_resource.m_msaa_image = std::make_shared<Image>(m_device, slot_name + "|msaa");
    }

    auto img_ci = tools::make_info<VkImageCreateInfo>({
        .imageType = VK_IMAGE_TYPE_2D,
        .format = m_format,
        .extent =
            {
                .width = m_width,
                .height = m_height,
                .depth = 1,
            },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = [&]() -> VkImageUsageFlags {
            switch (m_usage) {
            case TextureUsage::DEFAULT:
                return VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            case TextureUsage::COLOR_ATTACHMENT:
                return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            default:
                return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            }
        }(),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    });

    const auto img_view_ci = tools::make_info<VkImageViewCreateInfo>({
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = m_format,
        .subresourceRange =
            {
                .aspectMask = [&]() -> VkImageAspectFlags {
                    switch (m_usage) {
                    case TextureUsage::DEPTH_ATTACHMENT:
                        switch (m_format) {
                        case VK_FORMAT_D16_UNORM:
                        case VK_FORMAT_D32_SFLOAT:
                            return VK_IMAGE_ASPECT_DEPTH_BIT;
                        case VK_FORMAT_S8_UINT:
                            return VK_IMAGE_ASPECT_STENCIL_BIT;
                        case VK_FORMAT_D16_UNORM_S8_UINT:
                        case VK_FORMAT_D24_UNORM_S8_UINT:
                        case VK_FORMAT_D32_SFLOAT_S8_UINT:
                            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                        default:
                            return VK_IMAGE_ASPECT_DEPTH_BIT;
                        }
                    default:
                        return VK_IMAGE_ASPECT_COLOR_BIT;
                    }
                }(),
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    });

    frame_resource.m_image->create(img_ci, img_view_ci);
    frame_resource.m_descriptor_img_info = {
        .sampler = m_default_sampler->sampler(),
        .imageView = frame_resource.m_image->m_img_view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    if (m_samples > VK_SAMPLE_COUNT_1_BIT && frame_resource.m_msaa_image) {
        img_ci.samples = m_samples;
        frame_resource.m_msaa_image->create(img_ci, img_view_ci);
    }
}

Texture::PerFrameTextureResources &Texture::current_frame_resources() {
    return m_per_frame_texture_resources.at(m_current_frame_slot);
}

const Texture::PerFrameTextureResources &Texture::current_frame_resources() const {
    return m_per_frame_texture_resources.at(m_current_frame_slot);
}

void Texture::destroy_all() {
    for (auto &slot : m_per_frame_texture_resources) {
        destroy_per_frame_resources(slot);
    }
}

void Texture::destroy_per_frame_resources(PerFrameTextureResources &resources) {
    if (resources.m_image) {
        resources.m_image->destroy();
        resources.m_image.reset();
    }
    if (resources.m_msaa_image) {
        resources.m_msaa_image->destroy();
        resources.m_msaa_image.reset();
    }
    resources.m_descriptor_img_info = {};
}

VkImageView Texture::image_view() const {
    return current_frame_resources().m_image->image_view();
}

void Texture::prepare_initial_layout_barriers(PipelineBarrierBatchBuilder &barrier_builder) {
    if (m_src_texture_data_size != 0) {
        return;
    }

    const auto target_layout = [&]() -> VkImageLayout {
        switch (m_usage) {
        case TextureUsage::COLOR_ATTACHMENT:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case TextureUsage::DEPTH_ATTACHMENT:
        case TextureUsage::STENCIL_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        default:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }();

    if (target_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
        return;
    }

    const auto target_stage_mask = [&]() -> VkPipelineStageFlags2 {
        switch (target_layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        default:
            return VK_PIPELINE_STAGE_2_NONE;
        }
    }();

    const auto target_access_mask = [&]() -> VkAccessFlags2 {
        switch (target_layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        default:
            return VK_ACCESS_2_NONE;
        }
    }();

    const auto aspect_mask = [&]() -> VkImageAspectFlags {
        switch (m_usage) {
        case TextureUsage::DEPTH_ATTACHMENT:
            switch (m_format) {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_D32_SFLOAT:
                return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT);
            case VK_FORMAT_S8_UINT:
                return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_STENCIL_BIT);
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
            default:
                return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT);
            }
        case TextureUsage::STENCIL_ATTACHMENT:
            return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_STENCIL_BIT);
        default:
            return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }();

    for (const auto &slot : m_per_frame_texture_resources) {
        barrier_builder.add(tools::make_info<VkImageMemoryBarrier2>({
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstStageMask = target_stage_mask,
            .dstAccessMask = target_access_mask,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = target_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = slot.m_image->m_img,
            .subresourceRange =
                {
                    .aspectMask = aspect_mask,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        }));
    }
}

void Texture::prepare_update_barriers(PipelineBarrierBatchBuilder &barrier_builder) {
    if (m_src_texture_data_size == 0) {
        return;
    }

    const auto old_layout = [&]() {
        switch (m_usage) {
        case TextureUsage::COLOR_ATTACHMENT:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case TextureUsage::DEPTH_ATTACHMENT:
        case TextureUsage::STENCIL_ATTACHMENT:
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        default:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }();

    const auto src_stage_mask = [&]() -> VkPipelineStageFlags2 {
        switch (old_layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        default:
            return VK_PIPELINE_STAGE_2_NONE;
        }
    }();

    const auto src_access_mask = [&]() -> VkAccessFlags2 {
        switch (old_layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        default:
            return VK_ACCESS_2_NONE;
        }
    }();

    for (std::size_t slot_index = 0; slot_index < m_per_frame_texture_resources.size(); ++slot_index) {
        auto &slot = m_per_frame_texture_resources[slot_index];

        barrier_builder.add(tools::make_info<VkImageMemoryBarrier2>({
            .srcStageMask = src_stage_mask,
            .srcAccessMask = src_access_mask,
            .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .oldLayout = old_layout,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = slot.m_image->m_img,
            .subresourceRange =
                {
                    .aspectMask = [&]() -> VkImageAspectFlags {
                        switch (m_usage) {
                        case TextureUsage::DEPTH_ATTACHMENT:
                            switch (m_format) {
                            case VK_FORMAT_D16_UNORM:
                            case VK_FORMAT_D32_SFLOAT:
                                return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT);
                            case VK_FORMAT_S8_UINT:
                                return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_STENCIL_BIT);
                            case VK_FORMAT_D16_UNORM_S8_UINT:
                            case VK_FORMAT_D24_UNORM_S8_UINT:
                            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                                return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT |
                                                                       VK_IMAGE_ASPECT_STENCIL_BIT);
                            default:
                                return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_DEPTH_BIT);
                            }
                        case TextureUsage::STENCIL_ATTACHMENT:
                            return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_STENCIL_BIT);
                        default:
                            return static_cast<VkImageAspectFlags>(VK_IMAGE_ASPECT_COLOR_BIT);
                        }
                    }(),
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        }));
    }
}

void Texture::request_resize(const std::uint32_t width, const std::uint32_t height) {
    if (width == 0 || height == 0 || (width == m_width && height == m_height)) {
        return;
    }

    m_width = width;
    m_height = height;

    destroy_all();

    m_src_texture_data = nullptr;
    m_src_texture_data_size = 0;
    m_update_requested = true;
}

void Texture::request_update(void *src_texture_data, const std::size_t src_texture_data_size) {
    if (src_texture_data == nullptr || src_texture_data_size == 0) {
        return;
    }
    m_src_texture_data = src_texture_data;
    m_src_texture_data_size = src_texture_data_size;
    m_update_requested = true;
}

void Texture::set_frame_context(const std::size_t frame_slot_count, const std::size_t current_frame_slot) {
    const auto desired_slot_count = m_usage == TextureUsage::DEFAULT ? std::max<std::size_t>(1, frame_slot_count) : 1u;

    if (desired_slot_count < m_per_frame_texture_resources.size()) {
        for (std::size_t slot_index = desired_slot_count; slot_index < m_per_frame_texture_resources.size();
             ++slot_index) {
            destroy_per_frame_resources(m_per_frame_texture_resources[slot_index]);
        }
    }
    if (m_per_frame_texture_resources.size() != desired_slot_count) {
        m_per_frame_texture_resources.resize(desired_slot_count);
    }
    m_frame_slot_count = desired_slot_count;
    m_current_frame_slot = std::min(current_frame_slot, m_per_frame_texture_resources.size() - 1);
}

} // namespace inexor::vulkan_renderer::render_graph
