#pragma once

#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

namespace ae {

class sorter;

class container {
  friend class sorter;

public:
  using element_type = std::uint64_t;

  explicit container(std::span<const element_type> data, std::size_t num_threads);

  // TODO You may also add additional functions (or data members).

private:
  // TODO define your data layout
  // Your datastructure should consist of multiple blocks of data, which don't
  // necessarily have to be vectors.

  std::vector<std::vector<element_type>> blocks;
  std::array<std::vector<element_type>, 65> buckets;

public:
  [[nodiscard]] auto to_view() const { return std::views::join(blocks); }
};

} // namespace ae
