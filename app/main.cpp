#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <iterator>
#include <lab/memory/memory.hpp>
#include <map>
#include <memory>
#include <numeric>
#include <print>
#include <set>
#include <string>
#include <unordered_map>

namespace {

constexpr int kBytesToAllocate{64};
constexpr std::size_t kElementsCount{10};

}

auto main() -> int {
  try {
    lab::memory::PoolMemoryResource<kBytesToAllocate, kElementsCount> pool_resource{lab::memory::NewMemoryResource::Instance()};
    lab::memory::PoolAllocator<int> pool_allocator{&pool_resource};
    int* arr{pool_allocator.allocate(kElementsCount)};
    std::iota(arr, arr + kElementsCount, 0);
    std::for_each(arr, arr + kElementsCount, [counter = 0, arr](int& value) mutable -> void {
      arr[counter] = counter;
      std::println("arr[{}] = {}", counter, arr[counter]);
      ++counter;
    });
  } catch (const std::exception& error) {
    std::cerr << "Exception: " << error.what() << '\n';
    return 1;
  } catch (...) {
    std::cerr << "Unknown exception\n";
    return 1;
  }
  return 0;
}
