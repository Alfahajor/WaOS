#include "waos/memory/OptimalMemoryManager.h"
#include "waos/core/Process.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <queue>

void test_optimal_with_real_process() {
  std::cout << "[RUNNING] test_optimal_with_real_process..." << std::endl;
  
  uint64_t simulatedClock = 0;
  waos::memory::OptimalMemoryManager optimal(3, &simulatedClock);
  
  std::queue<waos::core::Burst> bursts;
  bursts.push({waos::core::BurstType::CPU, 10});
  
  auto process = std::make_unique<waos::core::Process>(1, 0, 1, bursts, 5);
  
  const auto& refString = process->getPageReferenceString();
  
  optimal.allocateForProcess(1, 5);
  optimal.registerFutureReferences(1, refString);
  
  simulatedClock = 1;
  int page0 = process->getCurrentPageRequirement();
  optimal.requestPage(1, page0);
  
  process->advanceInstructionPointer();
  optimal.advanceInstructionPointer(1);
  
  simulatedClock = 2;
  int page1 = process->getCurrentPageRequirement();
  optimal.requestPage(1, page1);
  
  process->advanceInstructionPointer();
  optimal.advanceInstructionPointer(1);
  
  simulatedClock = 3;
  int page2 = process->getCurrentPageRequirement();
  optimal.requestPage(1, page2);
  
  std::cout << "[PASSED] test_optimal_with_real_process" << std::endl;
}

void test_optimal_advantage() {
  std::cout << "[RUNNING] test_optimal_advantage..." << std::endl;
  
  uint64_t clock = 0;
  waos::memory::OptimalMemoryManager optimal(3, &clock);
  
  std::queue<waos::core::Burst> bursts;
  bursts.push({waos::core::BurstType::CPU, 7});
  
  auto process = std::make_unique<waos::core::Process>(2, 0, 1, bursts, 4);
  const auto& refs = process->getPageReferenceString();
  
  optimal.allocateForProcess(2, 4);
  optimal.registerFutureReferences(2, refs);
  
  for (size_t i = 0; i < std::min(size_t(7), refs.size()); ++i) {
    clock = i + 1;
    int page = process->getCurrentPageRequirement();
    optimal.requestPage(2, page);
    
    process->advanceInstructionPointer();
    optimal.advanceInstructionPointer(2);
  }
  
  std::cout << "[PASSED] test_optimal_advantage" << std::endl;
}

int main() {
  std::cout << "> Starting Optimal-Process Integration Tests" << std::endl;
  
  test_optimal_with_real_process();
  std::cout << std::endl;
  test_optimal_advantage();
  
  std::cout << "< All Integration Tests Passed" << std::endl;
  return 0;
}
