#pragma once

#include <vulkan/vulkan_core.h>

#include "inexor/vulkan-renderer/exception.hpp"

#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <span>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// RAII wrapper class for swapchains
class Swapchain {
private:
    Device &m_device;
    VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    std::optional<VkSurfaceFormatKHR> m_surface_format{};
    std::vector<VkImage> m_imgs;
    std::vector<VkImageView> m_img_views;
    std::uint32_t m_img_count{0};
    VkExtent2D m_extent{};

    /// Setup the swapchain
    /// @param width The width of the swapchain images
    /// @param height The height of the swapchain images
    /// @param old_swapchain The old swapchain which can be passed in to speed up swapchain recreation
    /// @exception VulkanException vkCreateSwapchainKHR call failed
    /// @exception VulkanException vkGetPhysicalDeviceSurfaceSupportKHR call failed
    void setup_swapchain(std::uint32_t width, std::uint32_t height, VkSwapchainKHR old_swapchain = VK_NULL_HANDLE);

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param surface The surface
    /// @param width The swapchain image width
    /// @param height The swapchain image height
    Swapchain(Device &device, VkSurfaceKHR surface, std::uint32_t width, std::uint32_t height);

    Swapchain(const Swapchain &) = delete;
    Swapchain(Swapchain &&) noexcept;

    ~Swapchain();

    Swapchain &operator=(const Swapchain &) = delete;
    Swapchain &operator=(Swapchain &&) = delete;

    /// Call vkAcquireNextImageKHR
    /// @param semaphore The semaphore to index signal (``VK_NULL_HANDLE`` by default)
    /// @param fence The fence to signal (``VK_NULL_HANDLE`` by default)
    /// @param timeout (``std::numeric_limits<std::uint64_t>::max()`` by default)
    /// @exception VulkanException vkAcquireNextImageKHR call failed
    /// @return The index of the next image
    [[nodiscard]] std::uint32_t
    acquire_next_image_index(VkSemaphore semaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE,
                             std::uint64_t timeout = std::numeric_limits<std::uint64_t>::max());

    /// Choose the composite alpha
    /// @param request_composite_alpha requested compositing flag
    /// @param supported_composite_alpha Alpha compositing modes supported on a device
    /// @exception std::runtime_error No compatible composite alpha could be found
    /// @return The chosen composite alpha flags
    [[nodiscard]] static std::optional<VkCompositeAlphaFlagBitsKHR>
    choose_composite_alpha(VkCompositeAlphaFlagBitsKHR request_composite_alpha,
                           VkCompositeAlphaFlagsKHR supported_composite_alpha);

    /// Choose the image array layer count
    /// @param requested_count The requested image array layer count
    /// @param max_count The maximum image array layer count
    /// @return The chosen image array layer count
    [[nodiscard]] static std::uint32_t choose_image_array_layer_count(std::uint32_t requested_count,
                                                                      std::uint32_t max_count);

    /// Determine the swapchain image extent
    /// @param requested_extent The image extent requested by the programmer
    /// @param min_extent The minimum extent
    /// @param max_extent The maximum extent
    /// @param current_extent The current extent
    /// @return The chosen swapchain image extent
    [[nodiscard]] static VkExtent2D choose_image_extent(const VkExtent2D &requested_extent,
                                                        const VkExtent2D &min_extent, const VkExtent2D &max_extent,
                                                        const VkExtent2D &current_extent);

    /// Choose the swapchain image count
    /// @param requested_count The requested swapchain image count
    /// @param min_count The minimum image count
    /// @param max_count The maximum image count
    /// @return The chosen swapchain image count
    [[nodiscard]] static std::uint32_t choose_image_count(std::uint32_t requested_count, std::uint32_t min_count,
                                                          std::uint32_t max_count);

    /// Choose the present mode
    /// @param available_present_modes The available present modes
    /// @param present_mode_priority_list The acceptable present modes (``DEFAULT_PRESENT_MODE_PRIORITY_LIST`` by
    /// default). Index ``0`` has highest priority, index ``n`` has lowest priority)
    /// @return The chosen present mode
    /// @note If none of the ``present_mode_priority_list`` are supported, ``VK_PRESENT_MODE_FIFO_KHR`` will be returned
    [[nodiscard]] static VkPresentModeKHR
    choose_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes,
                        const std::vector<VkPresentModeKHR> &present_mode_priority_list);

    /// Choose a surface format
    /// @param available_formats The available surface formats
    /// @param format_prioriy_list A priority list of acceptable surface formats (empty by default)
    /// @note Index ``0`` has highest priority, index ``n`` has lowest priority!
    /// @return The chosen surface format (``VK_FORMAT_UNDEFINED`` if no suitable format was found)
    [[nodiscard]] static std::optional<VkSurfaceFormatKHR>
    choose_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats,
                          const std::vector<VkSurfaceFormatKHR> &format_prioriy_list = {});

    /// Choose the surface transform
    /// @param request The requested surface transform
    /// @param supported The supported surface transform
    /// @param current The current surface transform
    /// @return The chosen image transform
    [[nodiscard]] static VkSurfaceTransformFlagBitsKHR choose_surface_transform(VkSurfaceTransformFlagBitsKHR requested,
                                                                                VkSurfaceTransformFlagsKHR supported,
                                                                                VkSurfaceTransformFlagBitsKHR current);

    [[nodiscard]] VkExtent2D extent() const {
        return m_extent;
    }

    [[nodiscard]] std::uint32_t image_count() const {
        return m_img_count;
    }

    [[nodiscard]] VkFormat image_format() const {
        return m_surface_format.value().format;
    }

    [[nodiscard]] const std::vector<VkImageView> &image_views() const {
        return m_img_views;
    }

    /// Check if a certain image usage flag is supported
    /// @param requested_flag The requested flag
    /// @param supported_flags The supported flags
    /// @return ``true`` if the requested image usage flag is supported
    [[nodiscard]] static bool is_image_usage_supported(VkImageUsageFlagBits requested_flag,
                                                       VkImageUsageFlags supported_flags);

    /// Call vkQueuePresentKHR
    /// @param img_index The image index
    /// @exception VulkanException vkQueuePresentKHR call failed
    void present(std::uint32_t img_index);

    /// Recreate the swapchain by calling ``setup_swapchain``
    /// @param width The swapchain image width
    /// @param height The swapchain image height
    void recreate(std::uint32_t width, std::uint32_t height);

    [[nodiscard]] const VkSwapchainKHR *swapchain() const {
        return &m_swapchain;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
