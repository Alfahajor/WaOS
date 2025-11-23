#pragma once

#include "PageTableEntry.h"
#include <unordered_map>

namespace waos::memory {

  /**
   * @brief Page Table type definition
   *
   * Maps logical page numbers to their corresponding page table entries
   * Each process has its own page table
   */
  using PageTable = std::unordered_map<int, PageTableEntry>;

}