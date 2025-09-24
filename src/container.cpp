#include "container.hpp"
#include <cassert>
#include <cstddef>
#include <span>
#include <vector>
#include <algorithm>

namespace ae {

int first_one_from_msb(uint64_t x) {
  if (x == 0)
    return -1;
  return std::bit_width(x) - 1;
}

static inline unsigned choose_B(size_t N, size_t threads) {
  if (N == 0) return 1;

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

  std::vector<std::vector<element_type>> buckets(num_buckets);
  for (element_type v : data) {
    const size_t prefix = (B == 64) ? 0u : size_t(v >> (64 - B));
    buckets[prefix].push_back(v);
  }

  blocks.clear();
  blocks.reserve(num_buckets);
  for (auto &bucket : buckets) {
    if (!bucket.empty()) blocks.emplace_back(std::move(bucket));
  }
}


} // namespace ae