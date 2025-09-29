#include "container.hpp"
#include <algorithm>
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

static inline unsigned choose_B(size_t N, size_t threads) {
  if (N == 0)
    return 1;

  const size_t target_bucket_size = 100'000;
  size_t buckets_by_size = (N + target_bucket_size - 1) / target_bucket_size;

  const size_t min_buckets = std::max<size_t>(threads * 8, 64);
  const size_t max_buckets = std::max<size_t>(threads * 64, 256);

  size_t buckets = std::clamp(buckets_by_size, min_buckets, max_buckets);
  buckets = std::bit_ceil(buckets);

  unsigned B = std::countr_zero(buckets);
  return std::min<unsigned>(B, 16);
}

container::container(std::span<const element_type> data, size_t num_threads) {
  const unsigned B = choose_B(data.size(), num_threads);
  const size_t num_buckets = size_t{1} << B;

  std::vector<element_type> buffer(data.size());
  std::vector<size_t> counts(num_buckets, 0);

  for (auto v : data) {
    size_t prefix = (B ? (size_t)(v >> (64 - B)) : 0);
    ++counts[prefix];
  }

  std::vector<size_t> starts(num_buckets);
  size_t sum = 0;
  for (size_t i = 0; i < num_buckets; ++i) {
    starts[i] = sum;
    sum += counts[i];
  }
  std::vector<size_t> write = starts;

  for (auto v : data) {
    size_t p = (B ? (size_t)(v >> (64 - B)) : 0);
    buffer[write[p]++] = v;
  }

  blocks.clear();
  blocks.reserve(num_buckets);
  for (size_t i = 0; i < num_buckets; ++i) {
    if (counts[i]) {
      blocks.emplace_back(buffer.begin() + starts[i],
                          buffer.begin() + starts[i] + counts[i]);
    }
  }
}

} // namespace ae