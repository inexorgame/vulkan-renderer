#pragma once

#include <volk.h>

#include <cstddef>
#include <functional>
#include <span>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

class FrameSyncManager {
private:
	const wrapper::Device &m_device;
	std::vector<VkFence> m_frame_slot_submission_fences{VK_NULL_HANDLE};
	std::size_t m_frame_slot_count{1};
	std::size_t m_current_frame_slot{0};

	struct DeferredRelease {
		std::vector<VkFence> fences;
		std::function<void()> release;
	};

	std::vector<DeferredRelease> m_deferred_releases;

public:
	explicit FrameSyncManager(const wrapper::Device &device);

	void set_frame_context(std::size_t frame_slot_count, std::size_t current_frame_slot);

	void mark_frame_slot_submission_fence(VkFence fence);

	void defer_release(std::span<const VkFence> fences, std::function<void()> release);

	void defer_release(VkFence fence, std::function<void()> release);

	void process_deferred_releases(bool wait_all);

	void clear();

	[[nodiscard]] const std::vector<VkFence> &frame_slot_submission_fences() const {
		return m_frame_slot_submission_fences;
	}

	[[nodiscard]] std::size_t frame_slot_count() const {
		return m_frame_slot_count;
	}

	[[nodiscard]] std::size_t current_frame_slot() const {
		return m_current_frame_slot;
	}
};

} // namespace inexor::vulkan_renderer::render_graph