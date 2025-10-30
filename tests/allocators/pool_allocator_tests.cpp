#include <gtest/gtest.h>

#include "inexor/vulkan-renderer/tools/allocators/pool_allocator.hpp"
#include "inexor/vulkan-renderer/tools/random.hpp"

#include <spdlog/spdlog.h>

#include <array>
#include <memory>

namespace inexor::vulkan_renderer::tools::allocators {

TEST(PoolAllocatorTests, Test1) {
    const std::size_t pool_size = 0;
    // This must throw an exception because you can't create a pool allocator of size 0.
    EXPECT_ANY_THROW(tools::allocators::PoolAllocator<std::uint32_t> numbers(pool_size));
}

TEST(PoolAllocatorTests, Test2) {
    const std::size_t pool_size = 1024;
    std::unique_ptr<PoolAllocator<std::uint32_t>> numbers;
    EXPECT_NO_THROW(numbers = std::make_unique<PoolAllocator<std::uint32_t>>(pool_size));
    // Check if the methods return the correct initial values.
    EXPECT_EQ(numbers->size(), pool_size);
    EXPECT_EQ(numbers->blocks_left_to_use(), pool_size);
    EXPECT_EQ(numbers->blocks_in_use(), 0);
}

TEST(PoolAllocatorTests, Test3) {
    const std::size_t pool_size = 1024;
    std::unique_ptr<PoolAllocator<std::uint32_t>> numbers;
    EXPECT_NO_THROW(numbers = std::make_unique<PoolAllocator<std::uint32_t>>(pool_size));
    // Take a slot from the pool allocator and fill it with a random number between 0 and 100.
    std::uint32_t *number = nullptr;
    EXPECT_NO_THROW(number = numbers->allocate(tools::generate_random_number(0, 100)));
    EXPECT_NE(number, nullptr);
    // Check the methods after requesting memory from the allocator.
    EXPECT_EQ(numbers->size(), pool_size);
    EXPECT_EQ(numbers->blocks_left_to_use(), pool_size - 1);
    EXPECT_EQ(numbers->blocks_in_use(), 1);
    // Free that block again.
    numbers->deallocate(number);
    EXPECT_EQ(numbers->size(), pool_size);
    EXPECT_EQ(numbers->blocks_left_to_use(), pool_size);
    EXPECT_EQ(numbers->blocks_in_use(), 0);
}

TEST(PoolAllocatorTests, Test4) {
    const std::size_t pool_size = 1024;
    tools::allocators::PoolAllocator<std::uint32_t> numbers(pool_size);
    // Check if the methods return the correct initial values.
    EXPECT_EQ(numbers.size(), pool_size);
    EXPECT_EQ(numbers.blocks_left_to_use(), pool_size);
    EXPECT_EQ(numbers.blocks_in_use(), 0);
    // Take a slot from the pool allocator and fill it with a random number between 0 and 100.
    const auto number = numbers.allocate(tools::generate_random_number(0, 100));
    // Check the methods after requesting memory from the allocator.
    EXPECT_EQ(numbers.size(), pool_size);
    EXPECT_EQ(numbers.blocks_left_to_use(), pool_size - 1);
    EXPECT_EQ(numbers.blocks_in_use(), 1);
    // Free that block again.
    numbers.deallocate(number);
    // The allocator should be in initial state again.
    EXPECT_EQ(numbers.size(), pool_size);
    EXPECT_EQ(numbers.blocks_left_to_use(), pool_size);
    EXPECT_EQ(numbers.blocks_in_use(), 0);
}

TEST(PoolAllocatorTests, Test5) {
    const std::size_t pool_size = 2;
    tools::allocators::PoolAllocator<std::uint32_t> numbers(pool_size);
    // Check if the methods return the correct initial values.
    EXPECT_EQ(numbers.size(), pool_size);
    EXPECT_EQ(numbers.blocks_left_to_use(), pool_size);
    EXPECT_EQ(numbers.blocks_in_use(), 0);
    // Take a slot from the pool allocator and fill it with two random number between 0 and 100.
    const std::array used_blocks{
        numbers.allocate(tools::generate_random_number(0, 100)),
        numbers.allocate(tools::generate_random_number(0, 100)),
    };
    // This must throw an exception because allocating a third block exceeds pool_size!
    EXPECT_ANY_THROW(const auto invalid = numbers.allocate(tools::generate_random_number(0, 100)));
    // Check the methods after requesting memory from the allocator.
    EXPECT_EQ(numbers.size(), pool_size);
    EXPECT_EQ(numbers.blocks_left_to_use(), pool_size - used_blocks.size());
    EXPECT_EQ(numbers.blocks_in_use(), used_blocks.size());
    // Free that block again.
    EXPECT_NO_THROW(numbers.deallocate(used_blocks[0]));
    EXPECT_NO_THROW(numbers.deallocate(used_blocks[1]));
    // The allocator should be in initial state again.
    EXPECT_EQ(numbers.size(), pool_size);
    EXPECT_EQ(numbers.blocks_left_to_use(), pool_size);
    EXPECT_EQ(numbers.blocks_in_use(), 0);
}

TEST(PoolAllocatorTests, Test6) {
    const std::size_t pool_size = 1024;
    tools::allocators::PoolAllocator<std::uint32_t> numbers(pool_size);
    // Check if the methods return the correct initial values.
    EXPECT_EQ(numbers.size(), pool_size);
    EXPECT_EQ(numbers.blocks_left_to_use(), pool_size);
    EXPECT_EQ(numbers.blocks_in_use(), 0);
    // An exception must be thrown if the pointer is nullptr!
    EXPECT_ANY_THROW(numbers.deallocate(nullptr));
    // An exception must be thrown if there iares no more blocks to be freed!
    std::uint32_t stack_variable = 0xdeadbeef;
    EXPECT_ANY_THROW(numbers.deallocate(&stack_variable));
    std::uint32_t *number1 = nullptr;
    // This must not throw an exception because we just allocating a new block.
    EXPECT_NO_THROW(number1 = numbers.allocate(tools::generate_random_number(0, 100)));
    // The allocator should be in initial state again.
    EXPECT_EQ(numbers.size(), pool_size);
    EXPECT_EQ(numbers.blocks_left_to_use(), pool_size - 1);
    EXPECT_EQ(numbers.blocks_in_use(), 1);
    // This must not throw an exception because we just free the block we took.
    EXPECT_NO_THROW(numbers.deallocate(number1));
    // The allocator should be in initial state again.
    EXPECT_EQ(numbers.size(), pool_size);
    EXPECT_EQ(numbers.blocks_left_to_use(), pool_size);
    EXPECT_EQ(numbers.blocks_in_use(), 0);
}

TEST(PoolAllocatorTests, Test7) {
    const std::size_t pool_size = 1024;
    tools::allocators::PoolAllocator<std::uint32_t> numbers(pool_size);
    // Check if the methods return the correct initial values.
    EXPECT_EQ(numbers.size(), pool_size);
    EXPECT_EQ(numbers.blocks_left_to_use(), pool_size);
    EXPECT_EQ(numbers.blocks_in_use(), 0);
    std::uint32_t *number1 = nullptr;
    std::uint32_t *number2 = nullptr;
    // This must not throw an exception because we just allocating a new block.
    EXPECT_NO_THROW(number1 = numbers.allocate(tools::generate_random_number(0, 100)));
    EXPECT_NO_THROW(number2 = numbers.allocate(tools::generate_random_number(0, 100)));
    // This must not throw an exception because we just free the block we took.
    EXPECT_NO_THROW(numbers.deallocate(number1));
    // Note that number1 is not set to nullptr here deliberately, because this would trigger another exception.
    // We allocated number2 to avoid triggering the exception that is thrown if no block are left to be freed.
    // This must trigger the double free exception.
    EXPECT_ANY_THROW(numbers.deallocate(number1));
    // Let's also free number2 to bring allocator back to initial state.
    EXPECT_NO_THROW(numbers.deallocate(number2));
    // Let's check this as well.
    EXPECT_ANY_THROW(numbers.deallocate(number2));
    EXPECT_ANY_THROW(numbers.deallocate(number2));
    // The allocator should be in initial state again.
    EXPECT_EQ(numbers.size(), pool_size);
    EXPECT_EQ(numbers.blocks_left_to_use(), pool_size);
    EXPECT_EQ(numbers.blocks_in_use(), 0);
}

} // namespace inexor::vulkan_renderer::tools::allocators
