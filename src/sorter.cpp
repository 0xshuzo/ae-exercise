#include "sorter.hpp"
#include "container.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace ae {

int sorter::start_bit_for_block(const std::vector<element_type> &a) {
  if (a.empty())
    return -1;
  auto [mn_it, mx_it] = std::minmax_element(a.begin(), a.end());
  element_type span = *mn_it ^ *mx_it;
  if (span == 0)
    return -1;
  return std::numeric_limits<element_type>::digits - 1 - std::countl_zero(span);
}

void sorter::sort(container &data) {
  for (auto &block : data.blocks) {
    // skip possibly empty blocks
    if (block.empty())
      continue;
    // search for first bit starting with MSD that is set
    const int bit0 = start_bit_for_block(block);
    if (bit0 >= 0)
      msd_radix_sort(block, 0, static_cast<int>(block.size()) - 1, bit0);
  }

}

void sorter::robin_hood_sort(std::vector<element_type> &block, int left,
                             int right) {
  const size_t n = static_cast<size_t>(right - left + 1);
  if (n == 0)
    return;
  const size_t buffer_size = std::max<size_t>(1, (5 * n + 1) / 2);
  element buffer[buffer_size];

  auto [min, max] =
      std::minmax_element(block.begin() + left, block.begin() + right + 1);
  if (*min == *max)
    return;

  for (int i = left; i < right + 1; i++) {
    size_t index = static_cast<size_t>((block[i] - *min) / (*max - *min) *
                                       (buffer_size - 1));

    element insert_element = {block[i], false};

    if (buffer[index].is_null)
      buffer[index] = insert_element;
    else {
      while (!buffer[index].is_null && block[i] > buffer[index].e) {
        index++;
      }
      while (!buffer[index].is_null) {
        std::swap(insert_element, buffer[index]);
        index++;
      }
      buffer[index] = insert_element;
    }
  }

  int pos = left;
  for (int i = 0; i < buffer_size; i++) {
    if (!buffer[i].is_null) {
      block[pos++] = buffer[i].e;
    }
  }
}

inline bool bit_test_mask(uint64_t x, uint64_t mask) noexcept {
  return (x & mask) != 0ULL;
}

void sorter::msd_radix_sort(std::vector<element_type> &block, int left,
                            int right, int bit) {
  if (left > right || bit < 0)
    return;

  if (right - left + 1 <= 32) {
    robin_hood_sort(block, left, right);
    return;
  }

  const uint64_t mask = (uint64_t{1} << bit);

  int i = left, j = right;
  while (i < j) {
    while (i < j && !ae::bit_test_mask(block[i], mask))
      ++i;
    while (i < j && ae::bit_test_mask(block[j], mask))
      --j;
    if (i < j)
      std::swap(block[i], block[j]);
  }

  const int split = i + (!ae::bit_test_mask(block[i], mask) ? 1 : 0);

  msd_radix_sort(block, left, split - 1, bit - 1);
  msd_radix_sort(block, split, right, bit - 1);
}

} // namespace ae
