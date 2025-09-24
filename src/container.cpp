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

  const size_t target_bucket_size = 100'000;     // 100k ist ein guter Startwert
  size_t buckets_by_size = (N + target_bucket_size - 1) / target_bucket_size;

  const size_t min_buckets = std::max<size_t>(threads * 8, 64);
  const size_t max_buckets = std::max<size_t>(threads * 64, 256);

  size_t buckets = std::clamp(buckets_by_size, min_buckets, max_buckets);
  // auf Potenz von 2 runden (für sauberes Bit-Präfix)
  buckets = std::bit_ceil(buckets);

  // B = log2(buckets); (buckets ist Potenz von 2)
  unsigned B = std::countr_zero(buckets);
  return std::min<unsigned>(B, 16);       // Sicherheitsdeckel gegen zu feine Körnung
}

container::container(std::span<const element_type> data, size_t num_threads) {
  const unsigned B = choose_B(data.size(), num_threads);
  const size_t BUCKETS = size_t{1} << B;

  std::vector<std::vector<element_type>> buckets(BUCKETS);
  for (element_type v : data) {
    const size_t prefix = (B == 64) ? 0u : size_t(v >> (64 - B));
    buckets[prefix].push_back(v);
  }

  blocks.clear();
  blocks.reserve(BUCKETS);
  for (auto &bucket : buckets) {
    if (!bucket.empty()) blocks.emplace_back(std::move(bucket));
  }
}


} // namespace ae