#pragma once

#include <stdexcept>
#include <memory>

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
            std::size_t m_size;

			PoolChunk<T> *m_data = nullptr;
            PoolChunk<T>* m_head = nullptr;

        public:
            explicit PoolAllocator(const std::size_t size = DEFAULT_SIZE) : m_size(size) {
                if (m_size == 0) {
                    throw std::invalid_argument("Error: size is 0!");
				}

				// This allocates memory, but does not call the constructor of T!
                m_data = new PoolChunk<T>[size];
                m_head = m_data;

				for (std::size_t index = 0; index < m_size-1; index++) {
					// We must use std::addressof because the & operator could be overloaded!
                    m_data[index].next_pool_chunk = std::addressof(m_data[index+1]);
				}
				m_data[size-1].next_pool_chunk = nullptr;
			}

			PoolAllocator(const PoolAllocator &other) = delete;
            PoolAllocator(PoolAllocator &&other) = delete;

			PoolAllocator &operator=(const PoolAllocator &other) = delete;
            PoolAllocator &operator=(PoolAllocator &&other) = delete;

			~PoolAllocator() {
                delete[] m_data;
                m_data = nullptr;
                m_head = nullptr;
			}

			template <typename ... arguments>
			[[nodiscard]] T *allocate(arguments && ... args) {
                if (m_head==nullptr) {
                    return nullptr;
				}
                auto *chunk = std::exchange(m_head, m_head->next_pool_chunk);
                // Placement new
                return new (std::addressof(chunk->value)) T(std::forward<arguments>(args)...);
            }

			void deallocate(T* data)
			{
				// TODO: Error check for data! Was this really the same type T?
                data->~T();
                auto pool_chunk = reinterpret_cast<PoolChunk<T>*>(data);
                pool_chunk->next_pool_chunk = m_head;
                m_head = pool_chunk;
			}

	};

}
