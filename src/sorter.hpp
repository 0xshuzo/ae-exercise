#pragma once

#include "container.hpp"
#include <vector>

namespace ae {

class sorter {
public:
  using element_type = std::uint64_t;

  struct element {
    element_type e;
    bool is_null = true;
  };

  void sort(container &data);

private:
  static int start_bit_for_block(const std::vector<element_type> &a);

  void msd_radix_sort(std::vector<element_type> &block, int left, int right,
                      int bit);

  void robin_hood_sort(std::vector<element_type> &block, int left, int right);
  // TODO You may add additional functions or data members to the sorter.
};

} // namespace ae
