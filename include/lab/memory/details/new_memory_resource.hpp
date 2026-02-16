#pragma once

#include <lab/memory/details/memory_resource.hpp>

namespace lab::memory {

class [[nodiscard]] NewMemoryResource : public MemoryResource {
  using Base = MemoryResource;

 protected:
  NewMemoryResource() = default;
  ~NewMemoryResource() = default;

 public:
  using size_type = Base::size_type;
  using SizeType = Base::SizeType;
  using difference_type = Base::difference_type;
  using DifferenceType = Base::DifferenceType;

  NewMemoryResource(const NewMemoryResource&) = delete;
  NewMemoryResource(NewMemoryResource&&) noexcept = delete;

  auto operator=(const NewMemoryResource&) -> NewMemoryResource& = delete;
  auto operator=(NewMemoryResource&&) noexcept -> NewMemoryResource& = delete;

  [[nodiscard]] static auto Instance() noexcept -> NewMemoryResource*;

  [[nodiscard]] auto Allocate(SizeType bytes) -> void* override;

  auto Deallocate(void* ptr, const SizeType bytes) -> void override;

  [[nodiscard]] auto IsEqual(const Base& other) const noexcept -> bool override;
};

}  // namespace lab::memory
