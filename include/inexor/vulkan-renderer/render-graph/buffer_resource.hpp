#pragma once

#include "inexor/vulkan-renderer/wrapper/buffer.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
/// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::descriptors {
/// Forward declaration
enum class DescriptorSetUpdateFrequency;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

using inexor::vulkan_renderer::wrapper::descriptors::DescriptorSetUpdateFrequency;

namespace inexor::vulkan_renderer::render_graph {

// Forward delcaration
class RenderGraph;

/// The buffer type describes the internal usage of the buffer resource inside of the rendergraph
enum class BufferType {
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
};

/// Wrapper for buffer resources inside of the rendergraph
class BufferResource {
    friend class RenderGraph;
    // CommandBuffer is allowed to access m_buffer when binding vertex and index buffers to make API nicer
    friend class wrapper::CommandBuffer;

private:
    /// The internal name of this buffer resource inside of the rendergraph
    std::string m_name;
    /// The buffer type
    BufferType m_type;
    /// The estimated buffer update frequency, which will be used if this is a uniform buffer resource and we
    /// need to group the descriptor for this uniform buffer into a descriptor set of the rendergraph which fits the
    /// update frequency (Note that descriptor sets should be grouped by udpate frequency for optimal performance)
    DescriptorSetUpdateFrequency m_update_frequency;
    /// An optional update function to update the data of the buffer resource
    std::optional<std::function<void()>> m_on_update{[]() {}};
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
    BufferResource(std::string name, BufferType type, DescriptorSetUpdateFrequency update_frequency,
                   std::optional<std::function<void()>> on_update);

    BufferResource(const BufferResource &) = delete;
    BufferResource(BufferResource &&other) noexcept;
    ~BufferResource() = default;

    BufferResource &operator=(const BufferResource &) = delete;
    BufferResource &operator=(BufferResource &&) = delete;

    /// Announce an update for this buffer resource
    /// @warning It is the responsibility of the caller to make absolutely sure that the memory is valid when the update
    /// is performec inside of rendergraph
    /// @note Enqueueing an update will not result in an immediate update of the buffer.
    /// Instead, the update will be carried out in update_dynamic_buffers() in the rendergraph.
    /// @param data A pointer to the data to copy the updated data from
    /// @param data_size The size of the data to copy
    void enqueue_update(void *data, const std::size_t data_size) {
        if (data == nullptr) {
            throw std::invalid_argument("Error: Buffer resource update failed (data pointer is nullptr)!");
        }
        if (data_size == 0) {
            throw std::invalid_argument("Error: Buffer resource update failed (data size is 0)!");
        }
        m_data = data;
        m_data_size = data_size;
        m_update_required = true;
    }

    /// Announce an update using a const reference to an instance of a type T
    template <typename BufferDataType>
    void enqueue_update(/*const?*/ BufferDataType &data) {
        return enqueue_update(&data, sizeof(data));
    }

    // Announce an update for data contained in a std::vector<T>
    template <typename BufferDataType>
    void enqueue_update(/*const?*/ std::vector<BufferDataType> &data) {
        return enqueue_update(data.data(), sizeof(data) * data.size());
    }

    // TODO: Implement a method for enforcing updates in mid-frame, like immediate_update?

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }

    [[nodiscard]] BufferType type() const {
        return m_type;
    }
};

} // namespace inexor::vulkan_renderer::render_graph
