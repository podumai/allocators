#pragma once

#include <cassert>
#include <cstddef>
#include <format>
#include <lab/memory/details/memory_resource.hpp>
#include <lab/memory/details/null_memory_resource.hpp>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

/**
 * @brief Namespace containing implementation of MemoryPool and PoolAllocator classes.
 * @namespace lab::memory
 */
namespace lab::memory {

/**
 * @brief Union that represents trivial building block.
 * @internal
 *
 * @tparam BlockSize The user required block size.
 *
 * @note The final block size is the max{BlockSize, sizeof(void*)}.
 */
template<std::size_t BlockSize>
requires (BlockSize > 0)
union Block final {
  Block* next_;
  char memory_[BlockSize]; // NOLINT: it is intended to be an contigious C-style array for memory reservation
};

/**
 * @brief Alias for representing owning MemoryRegion.
 * @internal
 * @typedef MemoryRegionType
 *
 * @tparam BlockType The type that represents block that will be mapped on raw memory.
 */
template<typename BlockType>
using MemoryRegionType = std::unique_ptr<BlockType[]>; // NOLINT: it is intended to be an contigious C-style array

/**
 * @brief Factory function for creating MemoryRegions.
 * @internal
 *
 * @tparam BlockType The type that represents block that will be mapped on raw memory.
 *
 * @param size The number of blocks to allocate.
 *
 * @see Block
 */
template<typename BlockType>
[[nodiscard]] auto MakeRegion(std::size_t const size) -> MemoryRegionType<BlockType> {
  return std::make_unique<BlockType[]>(size); // NOLINT: it is intended to be an contigious C-style array
}

/**
 * @brief The class that obtains pointers by chaining them into singly-linked list (View).
 * @internal
 *
 * @tparam BlockType The type that represents block that mapped on raw memory.
 *
 * @note BlockType must contain BlockType::next_ member.
 *       `FreeList` does not own the block sequence.
 *
 * @see BlockType
 */
template<typename BlockType>
requires (requires(BlockType block) {
  { block.next_ } -> std::same_as<BlockType*&>;
} && std::conjunction_v<std::negation<std::is_pointer<BlockType>>, std::negation<std::is_reference<BlockType>>>)
class [[nodiscard]] FreeList {
 public:
  using block_type = BlockType; // NOLINT: std like alias for block type
  using block_pointer = block_type*; // NOLINT: std like alias for block pointer type
  using BlockPointer = block_pointer; // NOLINT

  FreeList() noexcept = default;

  FreeList(const FreeList&) = delete;

  FreeList(FreeList&& other) noexcept : free_blocks_{std::exchange(other.free_blocks_, nullptr)} { }

  ~FreeList() = default;

  auto operator=(const FreeList&) -> FreeList& = delete;

  auto operator=(FreeList&& other) noexcept -> FreeList& {
    assert(this != &other);
    free_blocks_ = std::exchange(other.free_blocks_, nullptr);
    return *this;
  }

  /**
   * @brief Adds new block to the free list.
   * @internal
   * @public
   *
   * @param[in] block Block that will be added to the free list.
   *
   * @throws None (no-throw guarantee).
   */
  auto Push(BlockPointer block) noexcept -> void {
    assert(block);
    block->next_ = free_blocks_;
    free_blocks_ = block;
  }

  /**
   * @brief Pops one free block for new use.
   * @internal
   * @public
   *
   * @throws None (no-throw guarantee).
   *
   * @return BlockType* New free block.
   */
  [[nodiscard]] auto Pop() noexcept -> BlockPointer {
    assert(free_blocks_);
    BlockPointer const block{free_blocks_};
    free_blocks_ = free_blocks_->next_;
    return block;
  }

  /**
   * @brief Checks if the list is empty.
   * @internal
   * @public
   *
   * @throws None (no-throw guarantee).
   *
   * @return `true` if no blocks available, `false` otherwise.
   */
  [[nodiscard]] auto Empty() const noexcept -> bool { return !free_blocks_; }

  /**
   * @brief Clears the list of blocks.
   * @internal
   * @public
   *
   * @throws None (no-throw guarantee).
   *
   * @warning Available free blocks will not de deallocated.
   */
  auto Clear() noexcept -> void { free_blocks_ = nullptr; }

 private:
  BlockPointer free_blocks_{nullptr};
};

/**
 * @brief Class that represents pool allocation strategy with fixed size blocks.
 *
 * @tparam BlockSize The block size that will be used for pool creation.
 * @tparam BlocksPerRegion The number of blocks that one pool will contain.
 */
template<std::size_t BlockSize, std::size_t BlocksPerRegion>
requires (((BlockSize & 1) == 0) && BlockSize > 0 && BlocksPerRegion > 0)
class [[nodiscard]] PoolMemoryResource : virtual public AbstractMemoryResource {
  using BlockType = Block<BlockSize>;

  static constexpr std::size_t kInitialPoolCount{5};

 public:
  PoolMemoryResource() = default;

  explicit PoolMemoryResource(const std::size_t region_count, MemoryResource* resource) : regions_(region_count) {
    if (resource) {
      upstream_resource_ = resource;
    }

    if (!region_count) [[unlikely]] {
      return;
    }

    for (auto& region : regions_) {
      region = NewRegion();
    }
    free_list_.Push(regions_.front().get());

    constexpr std::size_t kLastBlock{BlocksPerRegion - 1};
    const std::size_t last_memory_region{region_count - 1};
    for (std::size_t i{}; i < last_memory_region; ++i) {
      regions_[i][kLastBlock].next_ = regions_[i + 1].get();
    }
  }

  explicit PoolMemoryResource(
    MemoryResource* upstream_resource
  ) noexcept(std::is_nothrow_default_constructible_v<decltype(regions_)>) {
    if (upstream_resource) [[likely]] {
      upstream_resource_ = upstream_resource;
    }
  }

  PoolMemoryResource(PoolMemoryResource const&) = delete;

  PoolMemoryResource(PoolMemoryResource&&) noexcept = default;

  ~PoolMemoryResource() override = default;

 private:
  /**
   * @brief Constructs new memory region and maps each block in singly-linked list.
   * @internal
   * @private
   *
   * @throws `std::bad_alloc` if memory allocation fails.
   *
   * @return MemoryRegionType<BlockType> New constructed memory region.
   */
  [[nodiscard]] static auto NewRegion() -> MemoryRegionType<BlockType> {
    auto region{MakeRegion<BlockType>(BlocksPerRegion)};
    for (std::size_t i{1}; i < BlocksPerRegion; ++i) {
      region[i].next_ = &region[i - 1]; // NOLINT: union can only contain POD
    }
    region[BlocksPerRegion - 1].next_ = nullptr; // NOLINT: union can only contain POD
    return region;
  }

 public:
  /**
   * @brief Allocates one fixed size block.
   * @public
   *
   * @throws `std::bad_alloc` if memory allocation fails.
   *
   * @return void* The pointer to the fixed size block.
   */
  [[nodiscard]] auto Allocate(SizeType bytes) -> void* override {
    if (!bytes) [[unlikely]] {
      ++bytes;
    }
    if (bytes > BlockSize) {
      return upstream_resource_->Allocate(bytes);
    }
    if (free_list_.Empty()) {
      regions_.push_back(NewRegion());
      free_list_.Push(regions_.back().get());
    }
    return reinterpret_cast<void*>(free_list_.Pop());
  }

  /**
   * @brief Memory that will be returned to the memory pool.
   * @public
   *
   * @throws None (no-throw guarantee).
   *
   * @warning **Undefined Behaviour** if:
   *   - The pointer passed does not belong to the memory pool.
   */
  auto Deallocate(void* ptr, const SizeType bytes) -> void override {
    if (bytes > BlockSize) {
      upstream_resource_->Deallocate(ptr, bytes);
      return;
    }
    free_list_.Push(reinterpret_cast<BlockType*>(ptr));
  }

  /**
   * @brief Releases resources obtained by memory pool.
   * @public
   *
   * @throws None (no-throw guarantee).
   */
  auto Release() noexcept -> void {
    regions_.clear();
    free_list_.Clear();
  }

  [[nodiscard]] auto IsEqual(const MemoryResource& memory_resource) const noexcept -> bool override {
    if (const PoolMemoryResource* pool_resource{dynamic_cast<const PoolMemoryResource*>(&memory_resource)};
        pool_resource) {
      return this == pool_resource;
    }
    return false;
  }

  auto operator=(PoolMemoryResource const&) -> PoolMemoryResource& = delete;

  auto operator=(PoolMemoryResource&&) noexcept -> PoolMemoryResource& = default;

 private:
  std::vector<MemoryRegionType<BlockType>> regions_;
  FreeList<BlockType> free_list_;
  MemoryResource* upstream_resource_{NewMemoryResource::Instance()};
};

/**
 * @brief Class that encapsulates logic of interacting with memory pool.
 *
 * @tparam T The type that fits in memory pool block.
 *
 * @note `PoolAllocator` class does not provide `void` overload.
 */
template<typename T>
class [[nodiscard]] PoolAllocator {
  template<typename>
  friend class PoolAllocator;

 public:
  // NOLINTBEGIN
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using is_always_equal = std::false_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_copy_construction = std::true_type;
  using propagate_on_container_swap = std::true_type;
  // NOLINTEND

  PoolAllocator() noexcept = default;

  /**
   * @brief Parametrized constructor for operating on pool instance.
   * @public
   *
   * @tparam BlockSize The block size that will be used for pool creation.
   * @tparam BlocksPerRegion The number of blocks that one pool will contain.
   *
   * @param[in] pool_resource Memory pool to allocate from.
   */
  template<std::size_t BlockSize, std::size_t BlocksPerRegion>
  PoolAllocator(PoolMemoryResource<BlockSize, BlocksPerRegion>* const pool_resource) noexcept
    : pool_resource_{pool_resource} { }

  template<typename U>
  PoolAllocator(const PoolAllocator<U>& other) noexcept : pool_resource_{other.GetPool()} { }

  PoolAllocator(const PoolAllocator& other) noexcept : pool_resource_{other.GetPool()} { }

  PoolAllocator(PoolAllocator&& other) noexcept = default;

  ~PoolAllocator() = default;

  /**
   * @brief Allocates one free block from underlying memory pool.
   * @public
   *
   * @param[in] n The value in set {0, 1}.
   *
   * @throws `std::bad_alloc` if memory allocation fails.
   *
   * @return `nullptr` if `n` is equal to zero, pointer to the requested memory otherwise.
   */
  [[nodiscard]] auto allocate(const size_type n) -> value_type* { // NOLINT
    return reinterpret_cast<value_type*>(pool_resource_->Allocate(n * sizeof(value_type)));
  }

  /**
   * @brief Returns memory to the underlying memory pool.
   * @public
   *
   * @param[in] ptr The pointer previosly allocated by `PoolAllocator`.
   * @param[in] n The number of blocks to deallocate (This value is ignored).
   *
   * @throws None (no-throw guarantee).
   *
   * @warning **Undefined Behaviour** if:
   *   - ptr does not belong to the underlying memory pool.
   */
  auto deallocate(value_type* const ptr, const size_type n) noexcept -> void { // NOLINT
    assert(pool_resource_);
    if (ptr) [[likely]] {
      pool_resource_->Deallocate(ptr, n * sizeof(value_type));
    }
  }

 protected:
  /**
   * @brief Gives access to the underlying memory pool.
   * @internal
   *
   * @throws None (no-throw guarantee).
   *
   * @return `MemoryPoolBase*` The pointer to the underlying memory pool.
   */
  [[nodiscard]] auto GetPool() const noexcept -> MemoryResource* { return pool_resource_; }

 public:
  auto operator=(const PoolAllocator& other) noexcept -> PoolAllocator& = default;

  auto operator=(PoolAllocator&& other) noexcept -> PoolAllocator& = default;

  template<typename U>
  [[nodiscard]] auto operator==(const PoolAllocator<U>& other) const noexcept -> bool {
    return pool_resource_ == other.GetPool();
  }

  template<typename U>
  [[nodiscard]] auto operator!=(const PoolAllocator<U>& other) const noexcept -> bool {
    return pool_resource_ != other.GetPool();
  }

 private:
  MemoryResource* pool_resource_{nullptr};
};

}  // namespace lab::memory
