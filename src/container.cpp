#include "container.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <span>
#include <vector>

namespace ae {

/**
  sources:

  ChatGPT (to find good values for bucket sizes and number of buckets to be used)
*/
static inline unsigned choose_B(size_t N, size_t threads) {
  if (N == 0)
    return 1;

  // want to target about 100,000 elements per block
  const size_t target_bucket_size = 100'000;
  size_t buckets_by_size = (N + target_bucket_size - 1) / target_bucket_size;

  // at least 8 buckets per thread but at least 64 in total
  const size_t min_buckets = std::max<size_t>(threads * 8, 64);
  // at most 64 buckets per thread but at least 256
  const size_t max_buckets = std::max<size_t>(threads * 64, 256);

  // get nearest value inside [min_buckets, max_buckets]
  size_t buckets = std::clamp(buckets_by_size, min_buckets, max_buckets);
  // round to next power of two for better use of bit operations
  buckets = std::bit_ceil(buckets);

  // B = log2(buckets)
  unsigned B = std::countr_zero(buckets);
  // upper bound of 2^16 elements to limit overhead
  return std::min<unsigned>(B, 16);
}

/**
  sources:

  own ideas (when thinking about how to split up work for multiple threads)

  ChatGPT (to efficiently find index errors
  produced during development)
*/
container::container(std::span<const element_type> data, size_t num_threads) {
  // choose number highest bits to consider to split up the data
  const unsigned B = choose_B(data.size(), num_threads);
  const size_t num_buckets = size_t{1} << B;

  std::vector<element_type> buffer(data.size());
  std::vector<size_t> counts(num_buckets, 0);

  for (auto v : data) {
    // mask out the B highest bit of an element v and increment the counter for
    // the resulting prefix
    size_t prefix = (B ? (size_t)(v >> (64 - B)) : 0);
    ++counts[prefix];
  }

  // determine the starting indices of the buckets
  std::vector<size_t> starts(num_buckets);
  size_t sum = 0;
  for (size_t i = 0; i < num_buckets; ++i) {
    starts[i] = sum;
    sum += counts[i];
  }
  std::vector<size_t> write = starts;

  // write elements to buffer w.r.t. the starting indices of the buckets
  for (auto v : data) {
    size_t p = (B ? (size_t)(v >> (64 - B)) : 0);
    buffer[write[p]++] = v;
  }

  // split buffer into real blocks w.r.t. to their starting indices and number
  // of elements per block
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