#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>

namespace lab::memory {

class MemoryResource {
 public:
  using size_type = std::size_t;
  using SizeType = size_type;
  using difference_type = std::ptrdiff_t;
  using DifferenceType = difference_type;

  MemoryResource() noexcept = default;
  MemoryResource(const MemoryResource&) = default;
  MemoryResource(MemoryResource&&) noexcept = default;
  virtual ~MemoryResource() = default;

  auto operator=(const MemoryResource&) -> MemoryResource& = default;
  auto operator=(MemoryResource&&) -> MemoryResource& = default;

  [[nodiscard]] virtual auto Allocate(const SizeType bytes) -> void* = 0;

  virtual auto Deallocate(void* ptr, const SizeType bytes) -> void = 0;

  [[nodiscard]] virtual auto IsEqual(const MemoryResource& other) const noexcept -> bool = 0;
};

}  // namespace lab::memory
