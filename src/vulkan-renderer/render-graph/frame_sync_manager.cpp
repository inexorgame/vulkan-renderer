#include "inexor/vulkan-renderer/render-graph/frame_sync_manager.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

FrameSyncManager::FrameSyncManager(const wrapper::Device &device) : m_device(device) {}

void FrameSyncManager::set_frame_context(const std::size_t frame_slot_count, const std::size_t current_frame_slot) {
	m_frame_slot_count = std::max<std::size_t>(1, frame_slot_count);
	m_current_frame_slot = std::min(current_frame_slot, m_frame_slot_count - 1);

	if (m_frame_slot_submission_fences.size() != m_frame_slot_count) {
		m_frame_slot_submission_fences.resize(m_frame_slot_count, VK_NULL_HANDLE);
	}
}

void FrameSyncManager::mark_frame_slot_submission_fence(const VkFence fence) {
	if (m_frame_slot_submission_fences.size() != m_frame_slot_count) {
		m_frame_slot_submission_fences.resize(m_frame_slot_count, VK_NULL_HANDLE);
	}
	m_frame_slot_submission_fences.at(m_current_frame_slot) = fence;
}

void FrameSyncManager::defer_release(const std::span<const VkFence> fences, std::function<void()> release) {
	if (!release) {
		return;
	}

	std::vector<VkFence> filtered_fences;
	filtered_fences.reserve(fences.size());
	for (const auto fence : fences) {
		if (fence != VK_NULL_HANDLE) {
			filtered_fences.push_back(fence);
		}
	}

	m_deferred_releases.push_back({
		.fences = std::move(filtered_fences),
		.release = std::move(release),
	});
}

void FrameSyncManager::defer_release(const VkFence fence, std::function<void()> release) {
	defer_release(std::span<const VkFence>{&fence, fence == VK_NULL_HANDLE ? 0u : 1u}, std::move(release));
}

void FrameSyncManager::process_deferred_releases(const bool wait_all) {
	if (m_deferred_releases.empty()) {
		return;
	}

	std::vector<VkFence> unique_fences;
	unique_fences.reserve(m_deferred_releases.size());
	std::unordered_set<VkFence> seen_fences;
	seen_fences.reserve(m_deferred_releases.size());

	for (const auto &entry : m_deferred_releases) {
		for (const auto fence : entry.fences) {
			if (fence == VK_NULL_HANDLE) {
				continue;
			}
			if (seen_fences.insert(fence).second) {
				unique_fences.push_back(fence);
			}
		}
	}

	if (wait_all) {
		if (!unique_fences.empty()) {
			if (const auto result = vkWaitForFences(m_device.device(), static_cast<std::uint32_t>(unique_fences.size()),
													unique_fences.data(), VK_TRUE,
													std::numeric_limits<std::uint64_t>::max());
				result != VK_SUCCESS) {
				throw VulkanException("Error: vkWaitForFences failed!", result, "FrameSyncManager::process_deferred_releases");
			}
		}

		for (auto &entry : m_deferred_releases) {
			entry.release();
		}
		m_deferred_releases.clear();
		return;
	}

	std::unordered_map<VkFence, bool> fence_ready;
	fence_ready.reserve(unique_fences.size());
	for (const auto fence : unique_fences) {
		const auto result = vkWaitForFences(m_device.device(), 1, &fence, VK_TRUE, 0);
		if (result == VK_SUCCESS) {
			fence_ready.emplace(fence, true);
		} else if (result == VK_TIMEOUT) {
			fence_ready.emplace(fence, false);
		} else {
			throw VulkanException("Error: vkWaitForFences failed!", result, "FrameSyncManager::process_deferred_releases");
		}
	}

	std::size_t write_index = 0;
	for (std::size_t read_index = 0; read_index < m_deferred_releases.size(); ++read_index) {
		auto &entry = m_deferred_releases[read_index];

		bool ready = true;
		for (const auto fence : entry.fences) {
			const auto it = fence_ready.find(fence);
			if (it == fence_ready.end() || !it->second) {
				ready = false;
				break;
			}
		}

		if (ready) {
			entry.release();
			continue;
		}

		m_deferred_releases[write_index++] = std::move(entry);
	}
	m_deferred_releases.resize(write_index);
}

void FrameSyncManager::clear() {
	m_deferred_releases.clear();
	m_frame_slot_submission_fences.clear();
	m_frame_slot_count = 1;
	m_current_frame_slot = 0;
}

} // namespace inexor::vulkan_renderer::render_graph