#include <algorithm>
#include <array>
#include <lab/list/list.hpp>
#include <lab/memory/memory.hpp>
#include <map>
#include <print>
#include <ranges>

namespace lab {

auto Task1() -> void;
auto Task2() -> void;

}  // namespace lab

auto main() -> int {
  lab::Task1();
  lab::Task2();
  return 0;
}

namespace lab {

inline namespace details {

auto PrintMap(const auto& map) -> void {
  for (auto&& [key, value] : map) {
    std::println("{} {}", std::forward<decltype(key)>(key), std::forward<decltype(value)>(value));
  }
}

constexpr std::array<int, 10> kFactorialValues{1, 1, 2, 6, 24, 120, 720, 5040, 40'320, 362'880};

auto GenerateMap(auto& map) -> void {
  for (auto&& [index, value] : std::views::enumerate(kFactorialValues)) {
    map.insert({std::forward<decltype(index)>(index), std::forward<decltype(value)>(value)});
  }
}

}  // namespace details

auto Task1() -> void {
  {
    std::map<int, int> m;

    GenerateMap(m);
    std::println("Task1[Standard Allocator]");
    PrintMap(m);
  }
  {
    using node_type = std::map<int, int>::node_type;
    constexpr std::size_t kNodeSize{sizeof(node_type)};
    using value_type = std::map<int, int>::value_type;

    lab::memory::PoolMemoryResource<kNodeSize, 10> pool;
    lab::memory::PoolAllocator<value_type> allocator{&pool};
    std::map<int, int, std::less<int>, decltype(allocator)> m{allocator};

    GenerateMap(m);
    std::println("Task1[Custom Allocator]");
    PrintMap(m);
  }
}

inline namespace details {

auto PrintList(const auto& list) -> void {
  auto begin{list.cbegin()};
  const auto end{list.cend()};
  while (begin != end) {
    std::print("{} ", *begin);
    ++begin;
  }
  std::println("");
}

auto GenerateList(auto& list) -> void {
  for (auto&& value : kFactorialValues) {
    list.PushFront(std::forward<decltype(value)>(value));
  }
}

}  // namespace details

auto Task2() -> void {
  {
    lab::containers::List<int> list;

    GenerateList(list);
    std::println("Task2[Standard Allocator]");
    PrintList(list);
  }
  {
    using NodeType = lab::containers::details::ListNode<int>;
    constexpr std::size_t kNodeTypeSize{sizeof(NodeType)};
    lab::memory::PoolMemoryResource<kNodeTypeSize, 10> pool;
    lab::memory::PoolAllocator<int> allocator{&pool};
    lab::containers::List<int, decltype(allocator)> list{allocator};

    GenerateList(list);
    std::println("Task2[Custom Allocator]");
    PrintList(list);
  }
}

}  // namespace lab
