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
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// RAII wrapper class for swapchains
class Swapchain {
    friend class render_graph::RenderGraph;

private:
    Device &m_device;
    VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    std::optional<VkSurfaceFormatKHR> m_surface_format{};
    std::vector<VkImage> m_imgs;
    std::vector<VkImageView> m_img_views;
    VkExtent2D m_extent{};
    std::unique_ptr<synchronization::Semaphore> m_img_available;
    bool m_vsync_enabled{false};
    std::uint32_t m_img_index;
    VkImage m_current_img{VK_NULL_HANDLE};
    VkImageView m_current_img_view{VK_NULL_HANDLE};

    /// Call vkGetSwapchainImagesKHR
    /// @exception inexor::vulkan_renderer::VulkanException vkGetSwapchainImagesKHR call failed
    /// @return A std::vector of swapchain images (this can be empty!)
    [[nodiscard]] std::vector<VkImage> get_swapchain_images();

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param surface The surface
    /// @param width The swapchain image width
    /// @param height The swapchain image height
    /// @param vsync_enabled ``true`` if vertical synchronization is enabled
    Swapchain(Device &device, VkSurfaceKHR surface, std::uint32_t width, std::uint32_t height, bool vsync_enabled);

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

    // TODO: Which ones should be expose in public?

    [[nodiscard]] VkExtent2D extent() const {
        return m_extent;
    }

    [[nodiscard]] const VkSemaphore *image_available_semaphore() const {
        return m_img_available->semaphore();
    }

    [[nodiscard]] VkImage image(const std::size_t img_index) const {
        return m_imgs.at(img_index);
    }

    [[nodiscard]] std::uint32_t image_count() const {
        return static_cast<std::uint32_t>(m_imgs.size());
    }

    [[nodiscard]] VkFormat image_format() const {
        return m_surface_format.value().format;
    }

    [[nodiscard]] const std::vector<VkImageView> &image_views() const {
        return m_img_views;
    }

    [[nodiscard]] VkImageView image_view(const std::size_t img_index) const {
        return m_img_views.at(img_index);
    }

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

    [[nodiscard]] const VkSwapchainKHR *swapchain() const {
        return &m_swapchain;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
