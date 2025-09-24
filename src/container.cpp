#include "container.hpp"
#include <cassert>
#include <cstddef>
#include <span>
#include <vector>

namespace ae {

int first_one_from_msb(uint64_t x) {
  if (x == 0)
    return -1;
  return std::bit_width(x) - 1;
}

container::container(std::span<const element_type> data) {
  for (element_type v : data) {
    int b = first_one_from_msb(v) + 1;
    buckets[static_cast<size_t>(b)].push_back(v);
  }

  blocks.clear();
  blocks.reserve(65);
  for (auto &bucket : buckets) {
    if (!bucket.empty()) {
      blocks.emplace_back(std::move(bucket));
    }
  }
}

} // namespace ae