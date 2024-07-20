#pragma once

#include "inexor/vulkan-renderer/wrapper/synchronization/semaphore.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::synchronization {
// Forward declaration
class Semaphore;
} // namespace inexor::vulkan_renderer::wrapper::synchronization

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
class GraphicsPass;
class GraphicsPassBuilder;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

// Using declarations
using commands::CommandBuffer;
using render_graph::GraphicsPass;
using render_graph::GraphicsPassBuilder;
using render_graph::RenderGraph;

/// RAII wrapper class for swapchains
class Swapchain {
    //
    friend class RenderGraph;
    friend class GraphicsPassBuilder;
    friend class GraphicsPass;

private:
    Device &m_device;
    std::string m_name;
    VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    std::optional<VkSurfaceFormatKHR> m_surface_format{};
    std::vector<VkImage> m_imgs;
    std::vector<VkImageView> m_img_views;
    VkExtent2D m_extent{};
    std::unique_ptr<synchronization::Semaphore> m_img_available;
    bool m_vsync_enabled{false};
    std::uint32_t m_img_index;
    VkImage m_current_swapchain_img{VK_NULL_HANDLE};
    VkImageView m_current_swapchain_img_view{VK_NULL_HANDLE};
    bool m_prepared_for_rendering{false};

    /// Call vkGetSwapchainImagesKHR
    /// @exception inexor::vulkan_renderer::VulkanException vkGetSwapchainImagesKHR call failed
    /// @return A std::vector of swapchain images (this can be empty!)
    [[nodiscard]] std::vector<VkImage> get_swapchain_images();

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The name of the swapchain
    /// @param surface The surface
    /// @param width The swapchain image width
    /// @param height The swapchain image height
    /// @param vsync_enabled ``true`` if vertical synchronization is enabled
    Swapchain(Device &device,
              std::string name,
              VkSurfaceKHR surface,
              std::uint32_t width,
              std::uint32_t height,
              bool vsync_enabled);

    Swapchain(const Swapchain &) = delete;
    Swapchain(Swapchain &&) noexcept;

    ~Swapchain();

    Swapchain &operator=(const Swapchain &) = delete;
    Swapchain &operator=(Swapchain &&) = delete;

    /// Call vkAcquireNextImageKHR
    /// @param timeout (``std::numeric_limits<std::uint64_t>::max()`` by default)
    /// @exception VulkanException vkAcquireNextImageKHR call failed
    void acquire_next_image_index(std::uint64_t timeout = std::numeric_limits<std::uint64_t>::max());

    /// Choose the composite alpha
    /// @param request_composite_alpha requested compositing flag
    /// @param supported_composite_alpha Alpha compositing modes supported on a device
    /// @exception std::runtime_error No compatible composite alpha could be found
    /// @return The chosen composite alpha flags
    [[nodiscard]] static std::optional<VkCompositeAlphaFlagBitsKHR>
    choose_composite_alpha(VkCompositeAlphaFlagBitsKHR request_composite_alpha,
                           VkCompositeAlphaFlagsKHR supported_composite_alpha);

    /// Determine the swapchain image extent
    /// @param requested_extent The image extent requested by the programmer
    /// @param min_extent The minimum extent
    /// @param max_extent The maximum extent
    /// @param current_extent The current extent
    /// @return The chosen swapchain image extent
    [[nodiscard]] static VkExtent2D choose_image_extent(const VkExtent2D &requested_extent,
                                                        const VkExtent2D &min_extent,
                                                        const VkExtent2D &max_extent,
                                                        const VkExtent2D &current_extent);

    /// Choose the present mode
    /// @param available_present_modes The available present modes
    /// @param present_mode_priority_list The acceptable present modes (``DEFAULT_PRESENT_MODE_PRIORITY_LIST`` by
    /// @param vsync_enabled ``true`` if vertical synchronization is enabled
    /// default). Index ``0`` has highest priority, index ``n`` has lowest priority)
    /// @return The chosen present mode
    /// @note If none of the ``present_mode_priority_list`` are supported, ``VK_PRESENT_MODE_FIFO_KHR`` will be returned
    [[nodiscard]] static VkPresentModeKHR
    choose_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes,
                        const std::vector<VkPresentModeKHR> &present_mode_priority_list,
                        bool vsync_enabled);

    /// Choose a surface format
    /// @param available_formats The available surface formats
    /// @param format_prioriy_list A priority list of acceptable surface formats (empty by default)
    /// @note Index ``0`` has highest priority, index ``n`` has lowest priority!
    /// @return The chosen surface format (``VK_FORMAT_UNDEFINED`` if no suitable format was found)
    [[nodiscard]] static std::optional<VkSurfaceFormatKHR>
    choose_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats,
                          const std::vector<VkSurfaceFormatKHR> &format_prioriy_list = {});

    [[nodiscard]] VkExtent2D extent() const {
        return m_extent;
    }

    [[nodiscard]] std::uint32_t image_count() const {
        return static_cast<std::uint32_t>(m_imgs.size());
    }

    [[nodiscard]] VkFormat image_format() const {
        return m_surface_format.value().format;
    }

    /// Change the image layout with a pipeline barrier to prepare for rendering
    /// @param cmd_buf The command buffer used for recording
    void change_image_layout_to_prepare_for_rendering(const CommandBuffer &cmd_buf);

    /// Change the image layout with a pipeline barrier to prepare to call vkQueuePresentKHR
    /// @param cmd_buf The command buffer used for recording
    void change_image_layout_to_prepare_for_presenting(const CommandBuffer &cmd_buf);

    /// Call vkQueuePresentKHR with the current image index
    /// @exception VulkanException vkQueuePresentKHR call failed
    void present();

    /// Setup the swapchain
    /// @param width The width of the swapchain images
    /// @param height The height of the swapchain images
    /// @param vsync_enabled ``true`` if vertical synchronization is enabled
    /// @exception VulkanException vkCreateSwapchainKHR call failed
    /// @exception VulkanException vkGetPhysicalDeviceSurfaceSupportKHR call failed
    void setup(std::uint32_t width, std::uint32_t height, bool vsync_enabled);
};

} // namespace inexor::vulkan_renderer::wrapper
