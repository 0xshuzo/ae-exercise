#include "sorter.hpp"
#include "container.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <thread>
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

void sorter::sort(container &data, std::size_t num_threads) {
  if (num_threads == 1)
    // sequentially sort blocks
    sort_linear(data);
  else
    // sort blocks in parallel
    sort_parallel(data, num_threads);
}

void sorter::sort_linear(container &data) {
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

/**
  ChatGPT was used to get help with writing the code to spawn threads and giving
  them work to do since I didn't know how to do it (did not parallelize anything
  before in C++)
*/
void sorter::sort_parallel(container &data, std::size_t num_threads) {
  std::atomic<std::size_t> next{0};
  std::vector<std::thread> workers;
  workers.reserve(num_threads);

  for (std::size_t w = 0; w < num_threads; ++w) {
    workers.emplace_back([&] {
      for (;;) {
        const std::size_t i = next.fetch_add(1, std::memory_order_relaxed);
        if (i >= data.blocks.size())
          break;

        auto &block = data.blocks[i];
        // skip blocks that only contain a single element or if they are empty
        if (block.size() <= 1)
          continue;

        // determine the highest bit set
        const int bit0 = start_bit_for_block(block);
        if (bit0 >= 0)
          msd_radix_sort(block, 0, static_cast<int>(block.size()) - 1, bit0);
      }
    });
  }

  for (auto &th : workers)
    th.join();
}

/**
  sources:

  https://github.com/mlochbaum/rhsort (Algorithm section in README for the idea
  behind the algorithm)

  ChatGPT (to efficiently find index errors
  produced during development)
*/
void sorter::robin_hood_sort(std::vector<element_type> &block, int left,
                             int right) {
  const size_t n = static_cast<size_t>(right - left + 1);
  if (n == 0)
    return;
  // create buffer of size 2.5*n
  const size_t buffer_size = std::max<size_t>(1, (5 * n + 1) / 2);
  element buffer[buffer_size];

  auto [min, max] =
      std::minmax_element(block.begin() + left, block.begin() + right + 1);
  if (*min == *max)
    return;

  for (int i = left; i < right + 1; i++) {
    // index in buffer to write the element to, normalized w.r.t. the range of
    // the elements and the maximum/minimum
    size_t index = static_cast<size_t>((block[i] - *min) / (*max - *min) *
                                       (buffer_size - 1));

    // is_null is used as a workaround to know if some element is empty or not
    // inside the buffer
    element insert_element = {block[i], false};

    if (buffer[index].is_null)
      // index the element should be written to is empty, directly able to write
      // it to buffer
      buffer[index] = insert_element;
    else {
      // need to move elements s.t. element to be inserted can be inserted
      // w.r.t. the existing elements inside the buffer
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
      // write back all elements that are not null inside the buffer
      block[pos++] = buffer[i].e;
    }
  }
}

inline bool bit_test_mask(uint64_t x, uint64_t mask) noexcept {
  return (x & mask) != 0ULL;
}

/**
  sources:

  explanation on the exercise sheet

  https://en.wikipedia.org/wiki/Radix_sort (understanding the idea
  behind the algorithm)

  ChatGPT (to efficiently find index errors produced
  during development and to choose initial parameters to experiment with, e.g.
  point to switch to robin hood sort)
*/
void sorter::msd_radix_sort(std::vector<element_type> &block, int left,
                            int right, int bit) {
  if (left > right || bit < 0)
    return;

  if (right - left + 1 <= 16) {
    robin_hood_sort(block, left, right);
    return;
  }

  const uint64_t mask = (uint64_t{1} << bit);

  int i = left, j = right;
  // until left and right meet
  while (i < j) {
    // search for two elements to be swapped
    while (i < j && !ae::bit_test_mask(block[i], mask))
      ++i;
    while (i < j && ae::bit_test_mask(block[j], mask))
      --j;
    // swap them if left and right didn't meet
    if (i < j)
      std::swap(block[i], block[j]);
  }

  // we don't know if the border between bit-th bit of elements in the block is
  // at i or i+1
  const int split = i + (!ae::bit_test_mask(block[i], mask) ? 1 : 0);

  // recursively sort elements with bit-th bit equal to 0 and equal to 1 using
  // the (bit - 1)-th bit
  msd_radix_sort(block, left, split - 1, bit - 1);
  msd_radix_sort(block, split, right, bit - 1);
}

} // namespace ae
