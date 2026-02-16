#include <lab/memory/details/new_memory_resource.hpp>

namespace lab::memory {

auto NewMemoryResource::Instance() noexcept -> NewMemoryResource* {
  static NewMemoryResource new_memory_resource{};
  return &new_memory_resource;
}

auto NewMemoryResource::Allocate(SizeType bytes) -> void* {
  if (!bytes) [[unlikely]] {
    ++bytes;
  }
  return ::operator new(bytes);
}

auto NewMemoryResource::Deallocate(void* ptr, const SizeType bytes) -> void { ::operator delete(ptr, bytes); }

auto NewMemoryResource::IsEqual(const Base& other) const noexcept -> bool {
  return dynamic_cast<decltype(this)>(&other);
}

}  // namespace lab::memory
