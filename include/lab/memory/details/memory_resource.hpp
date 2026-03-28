#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>

namespace lab::memory {

class AbstractMemoryResource {
 public:
  // NOLINTBEGIN
  using size_type = std::size_t;
  using SizeType = size_type;
  using difference_type = std::ptrdiff_t;
  using DifferenceType = difference_type;
  // NOLINTEND

  AbstractMemoryResource() noexcept = default;
  AbstractMemoryResource(const AbstractMemoryResource&) = default;
  AbstractMemoryResource(AbstractMemoryResource&&) noexcept = default;
  virtual ~AbstractMemoryResource() = default;

  auto operator=(const AbstractMemoryResource&) -> AbstractMemoryResource& = default;
  auto operator=(AbstractMemoryResource&&) -> AbstractMemoryResource& = default;

  [[nodiscard]] virtual auto Allocate(const SizeType bytes) -> void* = 0;

  virtual auto Deallocate(void* ptr, const SizeType bytes) -> void = 0;

  [[nodiscard]] virtual auto IsEqual(const AbstractMemoryResource& other) const noexcept -> bool = 0;
};

}  // namespace lab::memory
