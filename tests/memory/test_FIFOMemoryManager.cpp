#include "waos/memory/FIFOMemoryManager.h"
#include <cassert>
#include <iostream>

void test_basic_allocation() {
  std::cout << "[RUNNING] test_basic_allocation..." << std::endl;
  
  uint64_t simulatedClock = 0;
  waos::memory::FIFOMemoryManager fifo(4, &simulatedClock);
  
  fifo.allocateForProcess(1, 3);
  
  assert(!fifo.isPageLoaded(1, 0));
  assert(!fifo.isPageLoaded(1, 1));
  assert(!fifo.isPageLoaded(1, 2));
  
  std::cout << "[PASSED] test_basic_allocation" << std::endl;
}

void test_page_loading_no_replacement() {
  std::cout << "[RUNNING] test_page_loading_no_replacement..." << std::endl;
  
  uint64_t simulatedClock = 0;
  waos::memory::FIFOMemoryManager fifo(4, &simulatedClock);
  fifo.allocateForProcess(1, 3);
  
  fifo.requestPage(1, 0);
  assert(fifo.isPageLoaded(1, 0));
  
  fifo.requestPage(1, 1);
  assert(fifo.isPageLoaded(1, 1));
  
  fifo.requestPage(1, 2);
  assert(fifo.isPageLoaded(1, 2));
  
  assert(fifo.isPageLoaded(1, 0));
  assert(fifo.isPageLoaded(1, 1));
  assert(fifo.isPageLoaded(1, 2));
  
  std::cout << "[PASSED] test_page_loading_no_replacement" << std::endl;
}

void test_fifo_replacement() {
  std::cout << "[RUNNING] test_fifo_replacement..." << std::endl;
  
  uint64_t simulatedClock = 0;
  waos::memory::FIFOMemoryManager fifo(3, &simulatedClock);
  fifo.allocateForProcess(1, 4);
  
  fifo.requestPage(1, 0);
  fifo.requestPage(1, 1);
  fifo.requestPage(1, 2);
  
  assert(fifo.isPageLoaded(1, 0));
  assert(fifo.isPageLoaded(1, 1));
  assert(fifo.isPageLoaded(1, 2));
  
  // Load page 3 -> evicts page 0 (FIFO)
  fifo.requestPage(1, 3);
  
  assert(!fifo.isPageLoaded(1, 0));
  assert(fifo.isPageLoaded(1, 1));
  assert(fifo.isPageLoaded(1, 2));
  assert(fifo.isPageLoaded(1, 3));
  
  std::cout << "[PASSED] test_fifo_replacement" << std::endl;
}

void test_multiple_replacements() {
  std::cout << "[RUNNING] test_multiple_replacements..." << std::endl;
  
  uint64_t simulatedClock = 0;
  waos::memory::FIFOMemoryManager fifo(2, &simulatedClock);
  fifo.allocateForProcess(1, 4);
  
  // Reference: 0, 1, 2, 3, 0
  fifo.requestPage(1, 0);
  fifo.requestPage(1, 1);
  assert(fifo.isPageLoaded(1, 0) && fifo.isPageLoaded(1, 1));
  
  fifo.requestPage(1, 2);
  assert(!fifo.isPageLoaded(1, 0) && fifo.isPageLoaded(1, 1) && fifo.isPageLoaded(1, 2));
  
  fifo.requestPage(1, 3);
  assert(!fifo.isPageLoaded(1, 1) && fifo.isPageLoaded(1, 2) && fifo.isPageLoaded(1, 3));
  
  fifo.requestPage(1, 0);
  assert(fifo.isPageLoaded(1, 0) && !fifo.isPageLoaded(1, 2) && fifo.isPageLoaded(1, 3));
  
  std::cout << "[PASSED] test_multiple_replacements" << std::endl;
}

void test_process_deallocation() {
  std::cout << "[RUNNING] test_process_deallocation..." << std::endl;
  
  uint64_t simulatedClock = 0;
  waos::memory::FIFOMemoryManager fifo(4, &simulatedClock);
  
  fifo.allocateForProcess(1, 2);
  fifo.allocateForProcess(2, 2);
  
  fifo.requestPage(1, 0);
  fifo.requestPage(1, 1);
  fifo.requestPage(2, 0);
  fifo.requestPage(2, 1);
  
  fifo.freeForProcess(1);
  
  assert(fifo.isPageLoaded(2, 0));
  assert(fifo.isPageLoaded(2, 1));
  
  std::cout << "[PASSED] test_process_deallocation" << std::endl;
}

int main() {
  std::cout << "> Starting FIFO Memory Manager Tests" << std::endl;
  
  test_basic_allocation();
  std::cout << std::endl;
  test_page_loading_no_replacement();
  std::cout << std::endl;
  test_fifo_replacement();
  std::cout << std::endl;
  test_multiple_replacements();
  std::cout << std::endl;
  test_process_deallocation();
  
  std::cout << "< All FIFO Memory Manager Tests Passed" << std::endl;
  return 0;
}
