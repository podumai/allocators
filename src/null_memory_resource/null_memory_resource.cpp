#include <lab/memory/details/null_memory_resource.hpp>

namespace lab::memory {

auto NullMemoryResource::Instance() noexcept -> NullMemoryResource* {
  static NullMemoryResource null_memory_resource{};
  return &null_memory_resource;
}

auto NullMemoryResource::Allocate(SizeType /* bytes */) -> void* { throw std::bad_alloc{}; }

auto NullMemoryResource::Deallocate(void* /* ptr */, const SizeType /* bytes */) -> void { }

auto NullMemoryResource::IsEqual(const Base& other) const noexcept -> bool {
  return dynamic_cast<decltype(this)>(&other);
}

}  // namespace lab::memory
