import memory_pool;

#include <array>
#include <catch2/catch_test_macros.hpp>

namespace {

constexpr std::size_t kBlockSize{32};
constexpr std::size_t kBlocksPerRegion{5};

}  // namespace

TEST_CASE("MemoryPool Constructor [Zero init regions]") {
  lab::memory::MemoryPool<kBlockSize, kBlocksPerRegion> pool{0};
  std::array<void*, kBlocksPerRegion> pointers;
  for (void*& pointer : pointers) {
    pointer = pool.Allocate();
    REQUIRE(pointer);
  }
}

TEST_CASE("MemoryPool Constructor [Default Init regions]") {
  lab::memory::MemoryPool<128, kBlocksPerRegion> pool;
  std::array<void*, kBlocksPerRegion> pointers;
  for (auto& pointer : pointers) {
    pointer = pool.Allocate();
    REQUIRE(pointer);
  }
}

TEST_CASE("MemoryPool Block Reuse") {
  lab::memory::MemoryPool<kBlockSize, kBlocksPerRegion> pool;
  void* ptr{pool.Allocate()};
  pool.Deallocate(ptr);
  void* new_ptr{pool.Allocate()};
  CHECK(ptr == new_ptr);
}

TEST_CASE("PoolAllocator allocate method") {
  lab::memory::MemoryPool<kBlockSize, kBlocksPerRegion> pool{1};
  lab::memory::PoolAllocator<int> allocator{&pool};
  CHECK(allocator.allocate(0) == nullptr);
  REQUIRE(allocator.allocate(1));
}

TEST_CASE("PoolAllocator deallocate method") {
  lab::memory::MemoryPool<kBlockSize, kBlocksPerRegion> pool{1};
  lab::memory::PoolAllocator<int> allocator{&pool};
  int* location{allocator.allocate(1)};
  allocator.deallocate(location, 1);
  int* same_location{allocator.allocate(1)};
  REQUIRE(location == same_location);
}

TEST_CASE("PoolAllocator operator [Equal]") {
  lab::memory::PoolAllocator<int> allocator;
  {
    lab::memory::PoolAllocator<int> another_allocator;
    CHECK(allocator == another_allocator);
  }
  {
    lab::memory::PoolAllocator<unsigned long> another_allocator;
    CHECK(allocator == another_allocator);
  }
  lab::memory::MemoryPool<kBlockSize, kBlocksPerRegion> pool{1};
  {
    lab::memory::PoolAllocator<int> another_allocator{&pool};
    CHECK_FALSE(allocator == another_allocator);
  }
  {
    lab::memory::PoolAllocator<unsigned long> another_allocator{&pool};
    CHECK_FALSE(allocator == another_allocator);
  }
}

TEST_CASE("PoolAllocator operator [Not equal]") {
  {
    lab::memory::PoolAllocator<int> lhs;
    lab::memory::PoolAllocator<int> rhs;
    CHECK_FALSE(lhs != rhs);
  }
  lab::memory::MemoryPool<kBlockSize, kBlocksPerRegion> pool{1};
  lab::memory::PoolAllocator<int> lhs{&pool};
  {
    lab::memory::PoolAllocator<int> rhs;
    CHECK(lhs != rhs);
  }
  {
    lab::memory::PoolAllocator<int> rhs{&pool};
    CHECK_FALSE(lhs != rhs);
  }
  {
    lab::memory::PoolAllocator<long> rhs{&pool};
    CHECK_FALSE(lhs != rhs);
  }
}

TEST_CASE("PoolAllocator Copy constructor") {
  lab::memory::MemoryPool<kBlockSize, kBlocksPerRegion> pool{1};
  lab::memory::PoolAllocator<int> allocator{&pool};
  {
    auto another_allocator{allocator};
    CHECK(another_allocator == allocator);
  }
  {
    lab::memory::PoolAllocator<long> another_allocator{allocator};
    CHECK_FALSE(another_allocator != allocator);
  }
}
