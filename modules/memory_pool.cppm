module;

#include <cassert>
#include <cstddef>
#include <format>
#include <memory>
#include <new>
#include <stdexcept>
#include <tpu/modules/module_helper_macros.hpp>
#include <utility>
#include <vector>

export module memory_pool;

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
  char memory_[BlockSize];
};

/**
 * @brief Alias for representing owning MemoryRegion.
 * @internal
 * @typedef MemoryRegionType
 *
 * @tparam BlockType The type that represents block that will be mapped on raw memory.
 */
template<typename BlockType>
using MemoryRegionType = std::unique_ptr<BlockType[]>;

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
[[nodiscard]] auto MakeRegion(std::size_t size) -> MemoryRegionType<BlockType> {
  return std::make_unique<BlockType[]>(size);
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
class FreeList final {
 public:
  FreeList() noexcept = default;

  FreeList(FreeList&& other) noexcept : free_blocks_{std::exchange(other.free_blocks_, nullptr)} { }

  ~FreeList() = default;

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
  auto Push(BlockType* block) noexcept -> void {
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
  [[nodiscard]] auto Pop() noexcept -> BlockType* {
    assert(free_blocks_);
    BlockType* block{free_blocks_};
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
  BlockType* free_blocks_{nullptr};
};

template<typename T>
class PoolAllocator;

/**
 * @brief The base class that represents generic interface.
 * @internal
 */
class MemoryPoolBase {
 public:
  /**
   * @brief Returns allocated fixed size block.
   * @internal
   * @public
   *
   * @throws `std::bad_alloc` if allocation fails.
   *
   * @return void* The pointer to the fixed size block.
   */
  [[nodiscard]] virtual auto Allocate() -> void* = 0;

  /**
   * @brief Memory to return to the memory pool.
   * @internal
   * @public
   *
   * @throws None (no-throw guarantee).
   */
  virtual auto Deallocate(void*) noexcept -> void = 0;

  virtual ~MemoryPoolBase() = default;
};

START_EXPORT_SECTION

/**
 * @brief Namespace containing implementation of MemoryPool and PoolAllocator classes.
 * @namespace lab::memory
 */
namespace lab::memory {

/**
 * @brief Class that represents pool allocation strategy with fixed size blocks.
 *
 * @tparam BlockSize The block size that will be used for pool creation.
 * @tparam BlockPerRegion The number of blocks that one pool will contain.
 */
template<std::size_t BlockSize, std::size_t BlocksPerRegion>
requires (((BlockSize & 1) == 0) && BlockSize > 0 && BlocksPerRegion > 0)
class MemoryPool final : virtual public MemoryPoolBase {
  using BlockType = Block<BlockSize>;

  static constexpr std::size_t kInitialPoolCount{5};

 public:
  explicit MemoryPool(const std::size_t region_count = kInitialPoolCount) : regions_(region_count) {
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

  MemoryPool(MemoryPool&&) noexcept = default;

 private:
  /**
   * @brief Constructs new memory region and maps each block in signly-linked list.
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
      region[i].next_ = &region[i - 1];
    }
    region[BlocksPerRegion - 1].next_ = nullptr;
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
  [[nodiscard]] auto Allocate() -> void* override {
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
  auto Deallocate(void* ptr) noexcept -> void override { free_list_.Push(reinterpret_cast<BlockType*>(ptr)); }

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

  auto operator=(MemoryPool&&) noexcept -> MemoryPool& = default;

 private:
  std::vector<MemoryRegionType<BlockType>> regions_;
  FreeList<BlockType> free_list_;
};

/**
 * @brief Class that encapsulates logic of interacting with memory pool.
 *
 * @tparam T The type that fits in memory pool block.
 *
 * @note `PoolAllocator` class does not provide `void` overload.
 */
template<typename T>
class PoolAllocator {
  template<typename>
  friend class PoolAllocator;

 public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using is_always_equal = std::false_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_copy_construction = std::true_type;
  using propagate_on_container_swap = std::true_type;

  PoolAllocator() noexcept = default;

  /**
   * @brief Parametrisized constructor for operating on pool instance.
   * @public
   *
   * @param[in] pool Memory pool to allocate from.
   */
  explicit PoolAllocator(MemoryPoolBase* pool) noexcept : pool_{pool} { }

  PoolAllocator(const PoolAllocator& other) noexcept = default;

  template<typename U>
  PoolAllocator(const PoolAllocator<U>& other) noexcept : pool_{other.GetPool()} { }

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
  [[nodiscard]] auto allocate(const std::size_t n) const -> value_type* {
    assert(pool_ && n < 2);
    if (n) [[likely]] {
      return reinterpret_cast<value_type*>(pool_->Allocate());
    }
    return nullptr;
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
  auto deallocate(value_type* const ptr, [[maybe_unused]] const std::size_t n) const noexcept -> void {
    assert(pool_);
    if (ptr) [[likely]] {
      pool_->Deallocate(ptr);
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
  [[nodiscard]] auto GetPool() const noexcept -> MemoryPoolBase* { return pool_; }

 public:
  auto operator=(const PoolAllocator& other) noexcept -> PoolAllocator& = default;

  auto operator=(PoolAllocator&& other) noexcept -> PoolAllocator& = default;

  template<typename U>
  [[nodiscard]] auto operator==(const PoolAllocator<U>& other) const noexcept -> bool {
    return pool_ == other.GetPool();
  }

  template<typename U>
  [[nodiscard]] auto operator!=(const PoolAllocator<U>& other) const noexcept -> bool {
    return pool_ != other.GetPool();
  }

 private:
  MemoryPoolBase* pool_{nullptr};
};

}  // namespace lab::memory

END_EXPORT_SECTION
