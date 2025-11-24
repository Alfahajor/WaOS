/**
 * @brief Unit tests for Process Reference String generation and logic.
 */

#include "waos/core/Process.h"
#include <iostream>
#include <cassert>
#include <queue>
#include <set>

using namespace waos::core;

void test_reference_string_generation() {
  std::cout << "[RUNNING] test_reference_string_generation..." << std::endl;

  // Setup: Process requiring 5 pages, with 100 ticks of CPU
  std::queue<Burst> bursts;
  bursts.push({BurstType::CPU, 100});
  
  // PID 1, Arrival 0, 5 Pages
  Process p(1, 0, 0, bursts, 5);

  // Validate initial instruction pointer behavior
  int firstPage = p.getCurrentPageRequirement();
  assert(firstPage >= 0 && firstPage < 5);

  // Validate consistency and range over execution
  std::set<int> accessedPages;
  for (int i = 0; i < 100; ++i) {
    int page = p.getCurrentPageRequirement();
    
    // Check Range
    assert(page >= 0);
    assert(page < 5); // Must be strictly less than requiredPages

    accessedPages.insert(page);
    p.advanceInstructionPointer();
  }

  // With 100 ticks and 5 pages, randomness should likely touch multiple pages.
  // This is a probabilistic check, but with fixed seed in Process.cpp, it's deterministic.
  assert(accessedPages.size() > 1);

  std::cout << "[PASSED] test_reference_string_generation" << std::endl;
}

int main() {
  test_reference_string_generation();
  return 0;
}
