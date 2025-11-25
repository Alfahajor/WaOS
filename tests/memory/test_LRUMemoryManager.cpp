#include "waos/memory/LRUMemoryManager.h"
#include <cassert>
#include <iostream>

void test_basic_lru_replacement() {
  std::cout << "[RUNNING] test_basic_lru_replacement..." << std::endl;
  
  uint64_t simulatedClock = 0;
  waos::memory::LRUMemoryManager lru(3, &simulatedClock);
  lru.allocateForProcess(1, 4);
  
  simulatedClock = 1;
  lru.requestPage(1, 0);
  simulatedClock = 2;
  lru.requestPage(1, 1);
  simulatedClock = 3;
  lru.requestPage(1, 2);
  
  // Access page 0 again (updates to time 4)
  simulatedClock = 4;
  lru.requestPage(1, 0);
  
  // Load page 3 at time 5 -> evicts page 1 (LRU)
  simulatedClock = 5;
  lru.requestPage(1, 3);
  
  assert(lru.isPageLoaded(1, 0));
  assert(!lru.isPageLoaded(1, 1));
  assert(lru.isPageLoaded(1, 2));
  assert(lru.isPageLoaded(1, 3));
  
  std::cout << "[PASSED] test_basic_lru_replacement" << std::endl;
}

void test_access_pattern() {
  std::cout << "[RUNNING] test_access_pattern..." << std::endl;
  
  uint64_t simulatedClock = 0;
  waos::memory::LRUMemoryManager lru(3, &simulatedClock);
  lru.allocateForProcess(1, 4);
  
  // Reference: 0, 1, 2, 0, 1, 3
  simulatedClock = 1;
  lru.requestPage(1, 0);
  simulatedClock = 2;
  lru.requestPage(1, 1);
  simulatedClock = 3;
  lru.requestPage(1, 2);
  
  simulatedClock = 4;
  lru.requestPage(1, 0);
  
  simulatedClock = 5;
  lru.requestPage(1, 1);
  
  simulatedClock = 6;
  lru.requestPage(1, 3);
  
  assert(lru.isPageLoaded(1, 0));
  assert(lru.isPageLoaded(1, 1));
  assert(!lru.isPageLoaded(1, 2));
  assert(lru.isPageLoaded(1, 3));
  
  std::cout << "[PASSED] test_access_pattern" << std::endl;
}

void test_lru_vs_fifo_difference() {
  std::cout << "[RUNNING] test_lru_vs_fifo_difference..." << std::endl;
  
  uint64_t simulatedClock = 0;
  waos::memory::LRUMemoryManager lru(3, &simulatedClock);
  lru.allocateForProcess(1, 5);
  
  // Reference: 0, 1, 2, 1, 0, 3
  simulatedClock = 1;
  lru.requestPage(1, 0);
  simulatedClock = 2;
  lru.requestPage(1, 1);
  simulatedClock = 3;
  lru.requestPage(1, 2);
  simulatedClock = 4;
  lru.requestPage(1, 1);
  simulatedClock = 5;
  lru.requestPage(1, 0);
  
  simulatedClock = 6;
  lru.requestPage(1, 3);
  
  assert(lru.isPageLoaded(1, 0));
  assert(lru.isPageLoaded(1, 1));
  assert(!lru.isPageLoaded(1, 2));
  assert(lru.isPageLoaded(1, 3));
  
  std::cout << "[PASSED] test_lru_vs_fifo_difference" << std::endl;
}

void test_all_pages_fit() {
  std::cout << "[RUNNING] test_all_pages_fit..." << std::endl;
  
  uint64_t simulatedClock = 0;
  waos::memory::LRUMemoryManager lru(10, &simulatedClock);
  lru.allocateForProcess(1, 5);
  
  for (int i = 0; i < 5; ++i) {
    lru.requestPage(1, i);
  }
  
  for (int i = 0; i < 5; ++i) {
    assert(lru.isPageLoaded(1, i));
  }
  
  std::cout << "[PASSED] test_all_pages_fit" << std::endl;
}

void test_single_frame() {
  std::cout << "[RUNNING] test_single_frame..." << std::endl;
  
  uint64_t simulatedClock = 0;
  waos::memory::LRUMemoryManager lru(1, &simulatedClock);
  lru.allocateForProcess(1, 3);
  
  simulatedClock = 1;
  lru.requestPage(1, 0);
  assert(lru.isPageLoaded(1, 0));
  
  simulatedClock = 2;
  lru.requestPage(1, 1);
  assert(!lru.isPageLoaded(1, 0));
  assert(lru.isPageLoaded(1, 1));
  
  simulatedClock = 3;
  lru.requestPage(1, 2);
  assert(!lru.isPageLoaded(1, 1));
  assert(lru.isPageLoaded(1, 2));
  
  simulatedClock = 4;
  lru.requestPage(1, 2);
  assert(lru.isPageLoaded(1, 2));
  
  std::cout << "[PASSED] test_single_frame" << std::endl;
}

int main() {
  std::cout << "> Starting LRU Memory Manager Tests" << std::endl;
  
  test_basic_lru_replacement();
  std::cout << std::endl;
  test_access_pattern();
  std::cout << std::endl;
  test_lru_vs_fifo_difference();
  std::cout << std::endl;
  test_all_pages_fit();
  std::cout << std::endl;
  test_single_frame();
  
  std::cout << "< All LRU Memory Manager Tests Passed" << std::endl;
  return 0;
}
