#pragma once

#include <stdexcept>
#include <memory>
#include <cassert>
#include <mutex>
#include <shared_mutex>
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
            static constexpr std::size_t DEFAULT_SIZE{1024};
            std::size_t m_size = 0;
            std::size_t m_alloc_in_use = 0;

			PoolChunk<T> *m_data = nullptr;
            PoolChunk<T>* m_head = nullptr;

            std::shared_mutex m_mutex;

        public:
            explicit PoolAllocator(const std::size_t size = DEFAULT_SIZE) : m_size(size) {
                // Note that object construction is not concurrent: We assume that this is externally synchronized,
                // because we can't use an object until its constructor has finished anyways.
                // This means we do not need a lock guard here!
                if (m_size == 0) {
                    throw std::invalid_argument("Error: pool size is 0!");
				}
				// This allocates memory, but does not call the constructor of T!
                m_data = new PoolChunk<T>[size];
                // Point the head to the first index of the array.
                m_head = m_data;
                // Fill single linked list with data to the next pool chunk
				for (std::size_t index = 0; index < m_size-1; index++) {
					// We must use std::addressof because the & operator could be overloaded!
                    m_data[index].next_pool_chunk = std::addressof(m_data[index+1]);
				}
                // In the last pool chunk, set the data to the next pool chunk to nullptr.
				m_data[size-1].next_pool_chunk = nullptr;
			}

			PoolAllocator(const PoolAllocator &other) = delete;
            PoolAllocator(PoolAllocator &&other) = delete;

			PoolAllocator &operator=(const PoolAllocator &other) = delete;
            PoolAllocator &operator=(PoolAllocator &&other) = delete;

			~PoolAllocator() {
                std::unique_lock lock(m_mutex);
                // If this is not 0, we have allocations that are still in use and whose destructors have NOT been called yet!
                // Note that we can't throw an exception in a destructor, because std::terminate will be invoked!
                assert(m_alloc_in_use == 0 && "Error: Not all allocations in the pool allocator have been freed!");
                delete[] m_data;
                m_data = nullptr;
                m_head = nullptr;
			}

			template <typename ... arguments>
			[[nodiscard]] T *allocate(arguments && ... args) {
                std::unique_lock lock(m_mutex);
                if (m_head==nullptr) {
                    throw std::runtime_error("Error: Out of memory!");
				}
                m_alloc_in_use++;
                auto *chunk = std::exchange(m_head, m_head->next_pool_chunk);
                // Create the new object by using placement new and perfect forwarding.
                return new (std::addressof(chunk->value)) T(std::forward<arguments>(args)...);
            }

			void deallocate(T* data)
			{
                // Note that we need both reinterpret_cast here
                if (data > reinterpret_cast<T*>(m_data+m_size) || data < reinterpret_cast<T*>(m_data)) {
                    throw std::logic_error("Error: The deallocate() method was called with a pointer that was not allocated from the allocator!");
                }
                std::unique_lock lock(m_mutex);
                // Invoke the destructor of the allocated object manually.
                data->~T();
                m_alloc_in_use--;
                auto* pool_chunk = reinterpret_cast<PoolChunk<T>*>(data);
                pool_chunk->next_pool_chunk = m_head;
                m_head = pool_chunk;
			}

            [[nodiscard]] auto size() const noexcept {
                std::shared_lock lock(m_mutex);
                return m_size;
            }

	};

}
