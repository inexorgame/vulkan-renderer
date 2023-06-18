#pragma once

#include "inexor/vulkan-renderer/wrapper/buffer.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace inexor::vulkan_renderer::render_graph {

// Forward delcaration
class RenderGraph;
enum class DescriptorSetUpdateFrequencyCategory;

/// The usage of the buffer inside of the rendergraph
enum class BufferUsage {
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
};

/// Wrapper for buffer resources inside of the rendergraph
class BufferResource {
    friend RenderGraph;

private:
    /// The internal name of this buffer resource inside of the rendergraph
    std::string m_name;
    /// The internal buffer usage of this buffer resource inside of the rendergraph
    BufferUsage m_usage;
    /// The estimated buffer update frequency, which will be used if this is a uniform buffer resource and we
    /// need to group the descriptor for this uniform buffer into a descriptor set of the rendergraph which fits the
    /// update frequency (Note that descriptor sets should be grouped by udpate frequency for optimal performance)
    DescriptorSetUpdateFrequencyCategory m_update_frequency;
    /// An optional update function to update the data of the buffer resource
    std::function<void()> m_on_update{[]() {}};
    /// Indicates to the rendergraph if an update of this buffer resource has been announced
    bool m_update_required{false};
    /// If this is true, an update can only be carried out with the use of staging buffers
    bool m_requires_staging_buffer_update{false};
    /// The actual physical buffer which will be created by the rendergraph
    std::unique_ptr<wrapper::Buffer> m_buffer;

    // The data below is required for announcing updating to the buffer resource and for updating it in rendergraph
    // A raw pointer to the data to copy
    // The data will be copied in the update_dynamic_buffers() function inside of the rendergraph
    void *m_data{nullptr};
    // The size of the data to copy
    std::size_t m_data_size{0};

public:
    /// Default constructor
    /// @param name The internal debug name of the buffer (must not be empty)
    /// @param usage The internal usage of the buffer in the rendergraph
    /// @param update_frequency The estimated update frequency of this buffer
    /// @note The update frequency of a buffer will only be respected when grouping uniform buffers into descriptor sets
    /// @param on_update An optional update function (``std::nullopt`` by default, meaning no updates to this buffer)
    BufferResource(std::string name, BufferUsage usage, DescriptorSetUpdateFrequencyCategory update_frequency,
                   std::function<void()> on_update);

    BufferResource(const BufferResource &) = delete;
    BufferResource(BufferResource &&other) noexcept;
    ~BufferResource() = default;

    BufferResource &operator=(const BufferResource &) = delete;
    BufferResource &operator=(BufferResource &&) = delete;

    /// Announce an update for this buffer resource.
    /// @note Announcing an update will not result in an immediate update of the buffer.
    /// Instead, the update will be carried out in update_dynamic_buffers() in the rendergraph.
    /// @param data A pointer to the data to copy the updated data from
    /// @param data_size The size of the data to copy
    void announce_update(void *data, const std::size_t data_size) {
        if (data == nullptr) {
            throw std::invalid_argument("Error: Buffer resource update announce failed (data pointer is nullptr)!");
        }
        if (data_size == 0) {
            throw std::invalid_argument("Error: buffer resource update announce failed (data size is 0)!");
        }
        m_data = data;
        m_data_size = data_size;
        m_update_required = true;
    }

    /// Announce an update using a const reference to an instance of a type T
    template <typename T>
    void announce_update(const T &data) {
        return announce_update(&data, sizeof(T));
    }

    // Announce an update for data contained in a std::vector<T>
    template <typename T>
    void announce_update(const std::vector<T> &data) {
        return announce_update(data.data(), sizeof(T) * data.size());
    }
};

} // namespace inexor::vulkan_renderer::render_graph
