/**
 * @brief Integration tests for Simulator orchestration (CPU, IO, Page Faults).
 */

#include "waos/core/Simulator.h"
#include "waos/scheduler/IScheduler.h"
#include "waos/memory/IMemoryManager.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <map>
#include <cstdio>
#include <list>

using namespace waos::core;
using namespace waos::scheduler;
using namespace waos::memory;

class MockScheduler : public IScheduler {
public:
  std::list<Process*> readyQueue;

  void addProcess(Process* p) override {
    readyQueue.push_back(p);
  }

  Process* getNextProcess() override {
    if (readyQueue.empty()) return nullptr;
    Process* p = readyQueue.front();
    readyQueue.pop_front();
    return p;
  }

  bool hasReadyProcesses() const override {
    return !readyQueue.empty();
  }

  int getTimeSlice() const override { return -1; }
};

class MockMemoryManager : public IMemoryManager {
public:
  // Map to simulate physical memory: <PID, <Page, Loaded?>>
  std::map<int, std::map<int, bool>> memoryState;

  bool isPageLoaded(int pid, int page) const override {
    if (memoryState.count(pid) && memoryState.at(pid).count(page)) {
      return memoryState.at(pid).at(page);
    }
    return false; // Default: Not loaded
  }

  PageRequestResult requestPage(int pid, int page) override {
    // In a real system, this might trigger replacement.
    // In Mock, we just acknowledge the request. 
    // The test controller will manually flip the bool later to simulate load completion.
    return PageRequestResult::PAGE_FAULT;
  }

  void completePageLoad(int pid, int pageNumber) override {
    memoryState[pid][pageNumber] = true;
    std::cout << "MockMem: Page " << pageNumber << " loaded for PID " << pid << "\n";
  }

  void allocateForProcess(int pid, int pages) override { }
  
  void freeForProcess(int pid) override {
    memoryState.erase(pid);
  }
  
  // Helper for Test Controller
  void forceLoadPage(int pid, int page) {
    memoryState[pid][page] = true;
  }
};

void createTestFile(const std::string& fname, const std::string& content) {
  std::ofstream out(fname);
  out << content;
  out.close();
}

// Tests
void test_io_blocking_flow() {
  std::cout << "[RUNNING] test_io_blocking_flow..." << std::endl;
  std::string fname = "test_io.txt";
  createTestFile(fname, "P1 0 CPU(1),E/S(2),CPU(1) 1 4\n");

  Simulator sim;
  sim.loadProcesses(fname);
  
  auto sched = std::make_unique<MockScheduler>();
  auto mem = std::make_unique<MockMemoryManager>();
  auto memPtr = mem.get(); // Keep raw ptr to control mock
  
  sim.setScheduler(std::move(sched));
  sim.setMemoryManager(std::move(mem));
  
  // Pre-load all pages to avoid Page Faults in this test
  for(int i=0; i<4; ++i) memPtr->forceLoadPage(1, i);

  sim.start();

  sim.tick(); // 0: Arrival to Ready
  sim.tick(); // 1: CPU(1) finish to IO Start -> Blocked
  sim.tick(); // 2: IO(1/2)
  sim.tick(); // 3: IO(2/2) finish to Ready
  sim.tick(); // 4: Ready to CPU
  sim.tick(); // 5: CPU(1) finish to Terminated

  // Clean up
  sim.tick(); // Cleanup tick
  assert(!sim.isRunning()); // Should be finished

  std::cout << "[PASSED] test_io_blocking_flow" << std::endl;
  std::remove(fname.c_str());
}

void test_page_fault_auto_resolution() {
  std::cout << "[RUNNING] test_page_fault_flow..." << std::endl;
  std::string fname = "test_pf.txt";
  createTestFile(fname, "P1 0 CPU(10) 1 1\n");

  Simulator sim;
  sim.loadProcesses(fname);

  auto sched = std::make_unique<MockScheduler>();
  auto mem = std::make_unique<MockMemoryManager>();

  sim.setScheduler(std::move(sched));
  sim.setMemoryManager(std::move(mem));

  // DO NOT load pages. All pages are false.
  
  sim.start();

  // Tick 0: Arrival to Ready.
  sim.tick();

  // Tick 1: Scheduler picks P1 to Check Mem to False to Page Fault to Waiting Memory 
  sim.tick();

  assert(sim.isRunning());

  // Ticks 2, 3, 4, 5, 6: P1 is waiting.
  for(int i=0; i<5; ++i) sim.tick();

  // Tick 7: P1 moves from Wait to Ready.
  sim.tick();

  // Tick 8: Scheduler picks P1 to Memory Check (Now True) to CPU exec
  sim.tick();

  // Validate simulation is running (P1 is in CPU)
  assert(sim.isRunning());

  std::cout << "[PASSED] test_page_fault_flow" << std::endl;
  std::remove(fname.c_str());
}

int main() {
  test_io_blocking_flow();
  test_page_fault_auto_resolution();
  return 0;
}
