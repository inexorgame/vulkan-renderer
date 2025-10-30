#pragma once

#include <cassert>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::tools::allocators {

template <typename T>
union PoolChunk {
    T value;
    PoolChunk<T> *next_pool_chunk = nullptr;

    PoolChunk() {};
    ~PoolChunk() {};
};

template <typename T>
class PoolAllocator {
private:
    std::size_t m_size = 0;
    std::size_t m_blocks_in_use = 0;

    PoolChunk<T> *m_data = nullptr;
    PoolChunk<T> *m_head = nullptr;

    std::shared_mutex m_mutex;

public:
    explicit PoolAllocator(const std::size_t size) : m_size(size) {
        // We don't need a lock_guard here in the constructor!
        // Note that object construction is not concurrent: We assume that this is externally synchronized,
        // because we can't use an object until its constructor has finished anyways.
        if (m_size == 0) {
            throw std::invalid_argument("Error: pool size is 0!");
        }
        // This allocates memory, but does not call the constructor of T!
        // The RAII guard makes sure
        std::unique_ptr<PoolChunk<T>[]> scope_guard{new PoolChunk<T>[m_size]};
        // Retrieve the raw pointer
        m_data = scope_guard.get();
        // Point the head to the first index of the array.
        m_head = m_data;
        // Fill single linked list with data to the next pool chunk
        for (std::size_t index = 0; index < m_size - 1; index++) {
            // We must use std::addressof because the & operator could be overloaded!
            m_data[index].next_pool_chunk = std::addressof(m_data[index + 1]);
        }
        // In the last pool chunk, set the data to the next pool chunk to nullptr.
        m_data[size - 1].next_pool_chunk = nullptr;
        // Release ownership and do not call destructor of unique_ptr.
        scope_guard.release();
    }

    PoolAllocator(const PoolAllocator &other) = delete;
    PoolAllocator(PoolAllocator &&other) = delete;

    PoolAllocator &operator=(const PoolAllocator &other) = delete;
    PoolAllocator &operator=(PoolAllocator &&other) = delete;

    ~PoolAllocator() noexcept {
        // If this is not 0, we have allocations that are still in use and whose destructors have NOT been called yet!
        // Note that we can't throw an exception in a destructor, because std::terminate will be invoked!
        assert(m_blocks_in_use == 0 && "Error: Not all allocations in the pool allocator have been freed!");
        // @TODO Attempt to delete memory anyways
        delete[] m_data;
        m_data = nullptr;
        m_head = nullptr;
    }

    template <typename... arguments>
    [[nodiscard]] T *allocate(arguments &&...args) {
        // Unique lock because we need write access.
        std::unique_lock lock(m_mutex);
        // This is be equivalent to if(m_blocks_in_use==m_size)
        if (m_head == nullptr) {
            throw std::runtime_error("Error: Out of memory!");
        }
        auto *chunk = std::exchange(m_head, m_head->next_pool_chunk);
        try {
            // Placement new with perfect forwarding.
            T *ptr = new (std::addressof(chunk->value)) T(std::forward<arguments>(args)...);
            // There is now one more block in use.
            m_blocks_in_use++;
            return ptr;
        } catch (...) {
            chunk->next_pool_chunk = std::exchange(m_head, chunk);
            throw;
        }
    }

    [[nodiscard]] auto blocks_in_use() {
        // Shared lock because we don't need write access.
        std::shared_lock lock(m_mutex);
        return m_blocks_in_use;
    }

    [[nodiscard]] auto blocks_left_to_use() {
        return m_size - m_blocks_in_use;
    }

    void deallocate(T *data) {
        // Unique lock because we need write access.
        std::unique_lock lock(m_mutex);
        // Check if the pointer that was passed in points to valid memory to be freed.
        if (data == nullptr) {
            throw std::invalid_argument("Error: deallocate() was called with 'nullptr'!");
        }
        // Check if there are even any blocks left to be freed.
        if (m_blocks_in_use == 0) {
            throw std::logic_error("Error: deallocate() was called although there are no more blocks to free!");
        }
        // Byte based range check to avoid undefined behaviour (UB) when checking ranges with pointer arithmetic.
        const auto start = reinterpret_cast<std::uintptr_t>(m_data);
        const std::uintptr_t end = start + m_size * sizeof(PoolChunk<T>);
        const auto pointer = reinterpret_cast<std::uintptr_t>(data);
        // This range check would be UB because of overflow: if (data > m_data + m_size * sizeof(PoolChunk<T>))
        if (pointer < start || pointer > end) {
            throw std::logic_error("Error: deallocate() was called with a pointer that is out of memory range!");
        }
        // Check if he chunk size is correct.
        const auto chunk_size = sizeof(PoolChunk<T>);
        const auto offset = static_cast<std::size_t>(pointer - start);
        if ((offset % chunk_size) != 0) {
            throw std::logic_error("Error: deallocate() was called with a pointer of incorrect offset!");
        }
        // Check for double free mistakes.
        auto *pool_chunk = reinterpret_cast<PoolChunk<T> *>(data);
        for (auto *it = m_head; it != nullptr; it = it->next_pool_chunk) {
            if (it == pool_chunk) {
                throw std::logic_error("Error: Double free detected in deallocate()!");
            }
        }
        try {
            // @TODO Maybe we don't need to call the constructor for all types, what about e.g. std::uint32_t?
            // Invoke the destructor of the allocated object manually.
            std::destroy_at(std::addressof(pool_chunk->value));
        } catch (...) {
            // Do not throw an exception here!
        }
        // We now have one block less in use.
        m_blocks_in_use--;
        pool_chunk->next_pool_chunk = std::exchange(m_head, pool_chunk);
    }

    //@TODO Implement deallocate_all()!

    [[nodiscard]] auto size() {
        // Shared lock because we don't need write access.
        std::shared_lock lock(m_mutex);
        return m_size;
    }
};

} // namespace inexor::vulkan_renderer::tools::allocators
