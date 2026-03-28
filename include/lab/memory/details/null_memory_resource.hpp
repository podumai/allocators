#pragma once

#include <lab/memory/details/memory_resource.hpp>

namespace lab::memory {

class [[nodiscard]] NullMemoryResource : public AbstractMemoryResource {
  using Base = AbstractMemoryResource;

 protected:
  NullMemoryResource() noexcept = default;

 public:
  using size_type = Base::size_type; // NOLINT: std like alias for compatibility
  using SizeType = Base::SizeType;
  using difference_type = Base::difference_type; // NOLINT: std like alias for compatibility
  using DifferenceType = Base::DifferenceType;

  NullMemoryResource(const NullMemoryResource&) = delete;
  NullMemoryResource(NullMemoryResource&&) noexcept = delete;
  ~NullMemoryResource() override = default;

  auto operator=(const NullMemoryResource&) -> NullMemoryResource& = delete;
  auto operator=(NullMemoryResource&&) noexcept -> NullMemoryResource& = delete;

  [[nodiscard]] static auto Instance() noexcept -> NullMemoryResource*;

  [[nodiscard]] auto Allocate(SizeType bytes) -> void* override;

  auto Deallocate(void* ptr, const SizeType bytes) -> void override;

  [[nodiscard]] auto IsEqual(const Base& other) const noexcept -> bool override;
};

}  // namespace lab::memory
