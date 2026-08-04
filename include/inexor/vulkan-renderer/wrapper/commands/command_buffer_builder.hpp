#pragma once

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/per_frame_descriptor_sets.hpp"
#include "inexor/vulkan-renderer/wrapper/queries/query_pool.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::commands {

/// Builder for vkCmd-style command buffer recording operations.
class CommandBufferBuilder {
private:
    const CommandBuffer &m_command_buffer;

    [[nodiscard]] VkCommandBuffer command_buffer_handle() const {
        return m_command_buffer.command_buffer();
    }

public:
    explicit CommandBufferBuilder(const CommandBuffer &command_buffer) : m_command_buffer(command_buffer) {}

    [[nodiscard]] const CommandBuffer &command_buffer() const {
        return m_command_buffer;
    }

    [[nodiscard]] CommandBufferBuilder &begin_debug_label_region(const std::string &name, std::array<float, 4> color) {
        if (name.empty()) {
            throw InexorException("Error: Parameter 'name' is empty!");
        }
        auto label = tools::make_info<VkDebugUtilsLabelEXT>({
            .pLabelName = name.c_str(),
            .color = {color[0], color[1], color[2], color[3]},
        });
        vkCmdBeginDebugUtilsLabelEXT(command_buffer_handle(), &label);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &end_debug_label_region() {
        vkCmdEndDebugUtilsLabelEXT(command_buffer_handle());
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &insert_debug_label(const std::string &name, std::array<float, 4> color) {
        if (name.empty()) {
            throw InexorException("Error: Parameter 'name' is an empty string!");
        }
        auto label = tools::make_info<VkDebugUtilsLabelEXT>({
            .pLabelName = name.c_str(),
            .color = {color[0], color[1], color[2], color[3]},
        });
        vkCmdInsertDebugUtilsLabelEXT(command_buffer_handle(), &label);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &reset_query_pool(const queries::QueryPool &query_pool,
                                                         std::uint32_t first_query = 0, std::uint32_t query_count = 0) {
        if (query_pool.query_pool() == VK_NULL_HANDLE) {
            throw InexorException("Error: Parameter 'query_pool' is invalid!");
        }
        if (first_query >= query_pool.query_count()) {
            throw std::out_of_range("Error: Parameter 'first_query' is out of range!");
        }
        const auto actual_query_count = query_count == 0 ? query_pool.query_count() - first_query : query_count;
        if (actual_query_count == 0 || first_query + actual_query_count > query_pool.query_count()) {
            throw std::out_of_range("Error: Query range is out of range!");
        }
        vkCmdResetQueryPool(command_buffer_handle(), query_pool.query_pool(), first_query, actual_query_count);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &write_timestamp(const queries::QueryPool &query_pool, std::uint32_t query_index,
                                                        VkPipelineStageFlagBits stage_mask) {
        if (query_pool.query_pool() == VK_NULL_HANDLE) {
            throw InexorException("Error: Parameter 'query_pool' is invalid!");
        }
        if (query_index >= query_pool.query_count()) {
            throw std::out_of_range("Error: Parameter 'query_index' is out of range!");
        }
        vkCmdWriteTimestamp(command_buffer_handle(), stage_mask, query_pool.query_pool(), query_index);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &bind_descriptor_set(VkDescriptorSet descriptor_set,
                                                            std::weak_ptr<GraphicsPipeline> pipeline) {
        if (!descriptor_set) {
            throw InexorException("Error: Parameter 'descriptor_set' is invalid!");
        }
        const auto pipeline_ref = pipeline.lock();
        if (!pipeline_ref) {
            throw InexorException("Error: Parameter 'pipeline' is an invalid pointer!");
        }
        vkCmdBindDescriptorSets(command_buffer_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline_ref->pipeline_layout(), 0, 1, &descriptor_set, 0, nullptr);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &
    bind_descriptor_set(std::weak_ptr<descriptors::PerFrameDescriptorSets> descriptor_sets) {
        const auto desc_sets = descriptor_sets.lock();
        if (!desc_sets) {
            throw InexorException("Error: Parameter 'descriptor_set' is an invalid pointer!");
        }
        const VkDescriptorSet descriptor_set = desc_sets->current_descriptor_set();
        if (!descriptor_set) {
            throw InexorException("Error: Current descriptor set is invalid!");
        }

        const VkPipelineLayout pipeline_layout = desc_sets->pipeline_layout();
        if (!pipeline_layout) {
            throw InexorException("Error: Descriptor set pipeline layout is invalid!");
        }

        vkCmdBindDescriptorSets(command_buffer_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1,
                                &descriptor_set, 0, nullptr);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &
    bind_descriptor_set(std::weak_ptr<descriptors::PerFrameDescriptorSets> descriptor_sets,
                        std::weak_ptr<GraphicsPipeline> pipeline) {
        const auto descriptor_sets_ref = descriptor_sets.lock();
        if (!descriptor_sets_ref) {
            throw InexorException("Error: Parameter 'descriptor_set' is an invalid pointer!");
        }
        return bind_descriptor_set(descriptor_sets_ref->current_descriptor_set(), pipeline);
    }

    [[nodiscard]] CommandBufferBuilder &
    bind_descriptor_sets(std::span<const VkDescriptorSet> desc_sets, VkPipelineLayout layout,
                         VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS, std::uint32_t first_set = 0,
                         std::span<const std::uint32_t> dyn_offsets = {}) {
        if (!layout) {
            throw std::invalid_argument("Error: Parameter 'layout' is invalid!");
        }
        if (desc_sets.empty()) {
            throw std::invalid_argument("Error: Parameter 'desc_sets' is empty!");
        }
        vkCmdBindDescriptorSets(command_buffer_handle(), bind_point, layout, first_set,
                                static_cast<std::uint32_t>(desc_sets.size()), desc_sets.data(),
                                static_cast<std::uint32_t>(dyn_offsets.size()), dyn_offsets.data());
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &bind_index_buffer(std::weak_ptr<Buffer> buffer,
                                                          VkIndexType index_type = VK_INDEX_TYPE_UINT32,
                                                          VkDeviceSize offset = 0) {
        const auto buffer_ref = buffer.lock();
        if (!buffer_ref) {
            throw InexorException("Error: Parameter 'buffer' is an invalid pointer!");
        }
        if (buffer_ref->type() != BufferType::INDEX_BUFFER) {
            throw InexorException("Error: Rendergraph buffer resource " + buffer_ref->name() +
                                  " is not an index buffer!");
        }
        vkCmdBindIndexBuffer(command_buffer_handle(), buffer_ref->buffer(), offset, index_type);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &bind_index_buffer(VkBuffer buf, VkIndexType index_type = VK_INDEX_TYPE_UINT32,
                                                          VkDeviceSize offset = 0) {
        if (!buf) {
            throw std::invalid_argument("Error: Parameter 'buf' is invalid!");
        }
        vkCmdBindIndexBuffer(command_buffer_handle(), buf, offset, index_type);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &bind_pipeline(std::weak_ptr<GraphicsPipeline> graphics_pipeline) {
        const auto pipeline_ref = graphics_pipeline.lock();
        if (!pipeline_ref) {
            throw InexorException("Error: Parameter 'pipeline' is an invalid pointer!");
        }
        return bind_pipeline(pipeline_ref->pipeline());
    }

    [[nodiscard]] CommandBufferBuilder &
    bind_pipeline(VkPipeline pipeline, VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) {
        if (!pipeline) {
            throw std::invalid_argument("Error: Parameter 'pipeline' is invalid!");
        }
        vkCmdBindPipeline(command_buffer_handle(), bind_point, pipeline);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &bind_vertex_buffer(const std::weak_ptr<Buffer> buffer) {
        const auto buffer_ref = buffer.lock();
        if (!buffer_ref) {
            throw InexorException("Error: Parameter 'buffer' is an invalid pointer!");
        }
        if (buffer_ref->type() != BufferType::VERTEX_BUFFER) {
            throw InexorException("Error: Rendergraph buffer resource " + buffer_ref->name() +
                                  " is not a vertex buffer!");
        }
        const auto vk_buffer = buffer_ref->buffer();
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(command_buffer_handle(), 0, 1, &vk_buffer, &offset);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &bind_vertex_buffers(std::span<const VkBuffer> bufs,
                                                            std::uint32_t first_binding = 0,
                                                            std::span<const VkDeviceSize> offsets = {}) {
        if (bufs.empty()) {
            throw std::invalid_argument("Error: Parameter 'bufs' is empty!");
        }
        if (!offsets.empty()) {
            vkCmdBindVertexBuffers(command_buffer_handle(), first_binding, static_cast<std::uint32_t>(bufs.size()),
                                   bufs.data(), offsets.data());
            return *this;
        }
        constexpr std::size_t MAX_STACK_BINDINGS = 16;
        if (bufs.size() > MAX_STACK_BINDINGS) {
            throw std::out_of_range("Too many vertex buffer bindings for the fixed-size stack buffer!");
        }
        const std::array<VkDeviceSize, MAX_STACK_BINDINGS> zero_offsets{};
        vkCmdBindVertexBuffers(command_buffer_handle(), first_binding, static_cast<std::uint32_t>(bufs.size()),
                               bufs.data(), zero_offsets.data());
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &
    change_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,
                        VkImageSubresourceRange subres_range,
                        VkPipelineStageFlags src_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VkPipelineStageFlags dst_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) {
        if (image == VK_NULL_HANDLE) {
            throw std::invalid_argument("Error: Parameter 'image' is an invalid pointer!");
        }
        if (new_layout == old_layout) {
            throw std::invalid_argument("Error: old_layout and new_layout must differ!");
        }

        auto barrier = VkImageMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = static_cast<VkPipelineStageFlags2>(src_mask),
            .dstStageMask = static_cast<VkPipelineStageFlags2>(dst_mask),
            .oldLayout = old_layout,
            .newLayout = new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = subres_range,
        };

        switch (old_layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            barrier.srcAccessMask = 0;
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
            break;
        default:
            break;
        }

        switch (new_layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            if (barrier.srcAccessMask == 0) {
                barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
            }
            barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
            break;
        default:
            break;
        }

        return pipeline_image_memory_barrier(barrier);
    }

    [[nodiscard]] CommandBufferBuilder &
    change_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout,
                        std::uint32_t mip_level_count = 1, std::uint32_t array_layer_count = 1,
                        std::uint32_t base_mip_level = 0, std::uint32_t base_array_layer = 0,
                        VkPipelineStageFlags src_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VkPipelineStageFlags dst_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) {
        auto deduce_aspect_mask = [&]() -> VkImageAspectFlags {
            switch (format) {
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
                return VK_IMAGE_ASPECT_COLOR_BIT;
            }
        }();
        return change_image_layout(image, old_layout, new_layout,
                                   {
                                       .aspectMask = deduce_aspect_mask,
                                       .baseMipLevel = base_mip_level,
                                       .levelCount = mip_level_count,
                                       .baseArrayLayer = base_array_layer,
                                       .layerCount = array_layer_count,
                                   },
                                   src_mask, dst_mask);
    }

    [[nodiscard]] CommandBufferBuilder &copy_buffer(VkBuffer src_buf, VkBuffer dst_buf,
                                                    const VkBufferCopy &copy_region) {
        return copy_buffer(src_buf, dst_buf, {&copy_region, 1});
    }

    [[nodiscard]] CommandBufferBuilder &copy_buffer(VkBuffer src_buf, VkBuffer dst_buf,
                                                    std::span<const VkBufferCopy> copy_regions) {
        if (!src_buf) {
            throw std::invalid_argument("Error: Parameter 'src_buf' is invalid!");
        }
        if (!dst_buf) {
            throw std::invalid_argument("Error: Parameter 'dst_buf' is invalid!");
        }
        if (copy_regions.empty()) {
            throw std::invalid_argument("Error: Parameter 'copy_regions' is empty!");
        }
        vkCmdCopyBuffer(command_buffer_handle(), src_buf, dst_buf, static_cast<std::uint32_t>(copy_regions.size()),
                        copy_regions.data());
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &copy_buffer(VkBuffer src_buf, VkBuffer dst_buf, VkDeviceSize src_buf_size) {
        return copy_buffer(src_buf, dst_buf, {.size = src_buf_size});
    }

    [[nodiscard]] CommandBufferBuilder &copy_buffer_to_image(VkBuffer src_buf, VkImage dst_img,
                                                             std::span<const VkBufferImageCopy> copy_regions) {
        if (!src_buf) {
            throw std::invalid_argument("Error: Parameter 'src_buf' is invalid!");
        }
        if (!dst_img) {
            throw std::invalid_argument("Error: Parameter 'dst_img' is invalid!");
        }
        if (copy_regions.empty()) {
            throw std::invalid_argument("Error: Parameter 'copy_regions' is empty!");
        }
        vkCmdCopyBufferToImage(command_buffer_handle(), src_buf, dst_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               static_cast<std::uint32_t>(copy_regions.size()), copy_regions.data());
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &copy_buffer_to_image(VkBuffer src_buf, VkImage dst_img,
                                                             const VkBufferImageCopy &copy_region) {
        if (!src_buf) {
            throw std::invalid_argument("Error: Parameter 'src_buf' is invalid!");
        }
        if (!dst_img) {
            throw std::invalid_argument("Error: Parameter 'dst_img' is invalid!");
        }
        vkCmdCopyBufferToImage(command_buffer_handle(), src_buf, dst_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                               &copy_region);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &copy_buffer_to_image(VkBuffer buffer, VkImage img, VkExtent3D extent) {
        return copy_buffer_to_image(buffer, img,
                                    {
                                        .imageSubresource =
                                            {
                                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                .layerCount = 1,
                                            },
                                        .imageExtent =
                                            {
                                                .width = extent.width,
                                                .height = extent.height,
                                                .depth = 1,
                                            },
                                    });
    }

    [[nodiscard]] CommandBufferBuilder &copy_buffer_to_image(VkBuffer src_buf, std::weak_ptr<Image> img) {
        const auto image = img.lock();
        if (!image) {
            throw InexorException("Error: Parameter 'img' is an invalid pointer!");
        }
        return copy_buffer_to_image(src_buf, image->image(),
                                    {
                                        .width = image->width(),
                                        .height = image->height(),
                                        .depth = 1,
                                    });
    }

    [[nodiscard]] CommandBufferBuilder &draw(std::uint32_t vert_count, std::uint32_t inst_count = 1,
                                             std::uint32_t first_vert = 0, std::uint32_t first_inst = 0) {
        vkCmdDraw(command_buffer_handle(), vert_count, inst_count, first_vert, first_inst);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &draw_indexed(std::uint32_t index_count, std::uint32_t inst_count = 1,
                                                     std::uint32_t first_index = 0, std::int32_t vert_offset = 0,
                                                     std::uint32_t first_inst = 0) {
        vkCmdDrawIndexed(command_buffer_handle(), index_count, inst_count, first_index, vert_offset, first_inst);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &begin_rendering(const VkRenderingInfo &rendering_info) {
        vkCmdBeginRendering(command_buffer_handle(), &rendering_info);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &
    execute_secondary_command_buffers(std::span<const VkCommandBuffer> secondary_cmd_buffers) {
        if (secondary_cmd_buffers.empty()) {
            throw std::invalid_argument("Error: Parameter 'secondary_cmd_buffers' is empty!");
        }
        vkCmdExecuteCommands(command_buffer_handle(), static_cast<std::uint32_t>(secondary_cmd_buffers.size()),
                             secondary_cmd_buffers.data());
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &end_rendering() {
        vkCmdEndRendering(command_buffer_handle());
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &pipeline_barrier(const VkDependencyInfo &dependency_info) {
        vkCmdPipelineBarrier2(command_buffer_handle(), &dependency_info);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &
    pipeline_buffer_memory_barrier(const VkBufferMemoryBarrier2 &buffer_mem_barrier) {
        const auto dependency_info = tools::make_info<VkDependencyInfo>({
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &buffer_mem_barrier,
        });
        return pipeline_barrier(dependency_info);
    }

    [[nodiscard]] CommandBufferBuilder &pipeline_image_memory_barrier(const VkImageMemoryBarrier2 &img_barrier) {
        const auto dependency_info = tools::make_info<VkDependencyInfo>({
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &img_barrier,
        });
        return pipeline_barrier(dependency_info);
    }

    [[nodiscard]] CommandBufferBuilder &pipeline_memory_barrier(const VkMemoryBarrier2 &mem_barrier) {
        const auto dependency_info = tools::make_info<VkDependencyInfo>({
            .memoryBarrierCount = 1,
            .pMemoryBarriers = &mem_barrier,
        });
        return pipeline_barrier(dependency_info);
    }

    [[nodiscard]] CommandBufferBuilder &barrier_transfer_write_to_shader_read() {
        return pipeline_memory_barrier({
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
        });
    }

    [[nodiscard]] CommandBufferBuilder &barrier_color_attachment_write_to_shader_read() {
        return pipeline_memory_barrier({
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
        });
    }

    [[nodiscard]] CommandBufferBuilder &barrier_depth_stencil_write_to_shader_read() {
        return pipeline_memory_barrier({
            .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
        });
    }

    [[nodiscard]] CommandBufferBuilder &blit_image(VkImage src_image, VkImageLayout src_layout, VkImage dst_image,
                                                   VkImageLayout dst_layout, const VkImageBlit &blit,
                                                   VkFilter filter = VK_FILTER_LINEAR) {
        vkCmdBlitImage(command_buffer_handle(), src_image, src_layout, dst_image, dst_layout, 1, &blit, filter);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &push_constants(VkPipelineLayout layout, VkShaderStageFlags stage,
                                                       std::uint32_t size, const void *data, VkDeviceSize offset = 0) {
        if (!layout) {
            throw std::invalid_argument("Error: Parameter 'layout' is invalid!");
        }
        if (size == 0) {
            throw std::invalid_argument("Error: Parameter 'size' must be greater than zero!");
        }
        if (data == nullptr) {
            throw std::invalid_argument("Error: Parameter 'data' is invalid!");
        }
        vkCmdPushConstants(command_buffer_handle(), layout, stage, static_cast<std::uint32_t>(offset), size, data);
        return *this;
    }

    template <typename T>
    [[nodiscard]] CommandBufferBuilder &push_constant(const VkPipelineLayout layout, const T &data,
                                                      const VkShaderStageFlags stage, const VkDeviceSize offset = 0) {
        return push_constants(layout, stage, sizeof(data), &data, offset);
    }

    template <typename T>
    [[nodiscard]] CommandBufferBuilder &push_constant(const std::weak_ptr<GraphicsPipeline> pipeline, const T &data,
                                                      const VkShaderStageFlags stage, const VkDeviceSize offset = 0) {
        const auto pipeline_ref = pipeline.lock();
        if (!pipeline_ref) {
            throw InexorException("Error: Parameter 'pipeline' is an invalid pointer!");
        }
        return push_constants(pipeline_ref->pipeline_layout(), stage, sizeof(data), &data, offset);
    }

    [[nodiscard]] CommandBufferBuilder &set_scissor(VkRect2D scissor) {
        vkCmdSetScissor(command_buffer_handle(), 0, 1, &scissor);
        return *this;
    }

    [[nodiscard]] CommandBufferBuilder &set_viewport(VkViewport viewport) {
        vkCmdSetViewport(command_buffer_handle(), 0, 1, &viewport);
        return *this;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::commands
