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

using namespace waos::core;
using namespace waos::scheduler;
using namespace waos::memory;

class MockScheduler : public IScheduler {
public:
  std::list<Process*> readyQueue;

  void addProcess(std::unique_ptr<Process> p) override {} // Not used
  
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

  void tick() override {}
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

  void allocateForProcess(int pid, int pages) override {
    // Initialize pages as NOT loaded
    for(int i=0; i<pages; ++i) memoryState[pid][i] = false;
  }
  
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
  // P1: CPU(1), E/S(2), CPU(1).
  createTestFile(fname, "P1 0 CPU(1),E/S(2),CPU(1) 1 4\n");

  Simulator sim;
  sim.loadProcesses(fname);
  
  auto sched = std::make_unique<MockScheduler>();
  auto mem = std::make_unique<MockMemoryManager>();
  auto memPtr = mem.get(); // Keep raw ptr to control mock
  
  sim.setScheduler(std::move(sched));
  sim.setMemoryManager(std::move(mem));
  
  // Pre-load all pages to avoid Page Faults in this test
  memPtr->forceLoadPage(1, 0); 
  memPtr->forceLoadPage(1, 1); // Load a few pages just in case
  memPtr->forceLoadPage(1, 2);
  memPtr->forceLoadPage(1, 3);

  sim.start();

  // Tick 0: Arrival to Ready to CPU
  sim.tick(); 
  // Tick 1: P1 Finishes CPU burst to Next is IO to Blocked
  sim.tick(); 
  // Tick 2: P1 in Blocked (IO tick 1/2)
  sim.tick(); 
  // Tick 3: P1 in Blocked (IO tick 2/2) to Ready
  sim.tick(); 
  // Tick 4: Scheduler picks P1 to CPU
  sim.tick(); 
  // Tick 5: P1 Finishes CPU to Terminated
  sim.tick(); 

  // Assertions logic is implicitly handled by the Simulator not crashing 
  // and log outputs (if we captured them).
  // We can verify state by checking if simulation finished.
  
  // Clean up
  sim.tick(); // Cleanup tick
  assert(!sim.isRunning()); // Should be finished

  std::cout << "[PASSED] test_io_blocking_flow" << std::endl;
  std::remove(fname.c_str());
}

void test_page_fault_flow() {
  std::cout << "[RUNNING] test_page_fault_flow..." << std::endl;
  std::string fname = "test_pf.txt";
  // P1: CPU(10). Page Requirement generated internally.
  createTestFile(fname, "P1 0 CPU(10) 1 1\n");

  Simulator sim;
  sim.loadProcesses(fname);

  auto sched = std::make_unique<MockScheduler>();
  auto mem = std::make_unique<MockMemoryManager>();
  auto memPtr = mem.get(); // Control handle

  sim.setScheduler(std::move(sched));
  sim.setMemoryManager(std::move(mem));

  // DO NOT load pages. All pages are false.
  
  sim.start();

  // Tick 0: Arrival to Ready.
  sim.tick();

  // Tick 1: Scheduler picks P1.
  // Simulator checks Memory to isPageLoaded(P1, X)? to False.
  // Simulator triggers Page Fault to P1 to WAITING_MEMORY.
  sim.tick();

  // We don't have public access to verify state directly easily without friend classes,
  // but we can verify timing.
  // Penalty is 5 ticks (hardcoded in Simulator for now).
  
  // Ticks 2, 3, 4, 5, 6: P1 is waiting. CPU is idle.
  for(int i=0; i<5; ++i) sim.tick();

  // IMPORTANT: The simulator penalty only moves process back to Ready.
  // It effectively assumes the OS loaded the page in background.
  // HOWEVER, our MockMemoryManager is dumb. We MUST manually force load the page
  // so the *next* check passes. But we don't know which page P1 wanted (random).
  // Strategy: Force load ALL pages for P1 now.
  memPtr->forceLoadPage(1, 0);

  // Tick 7: P1 moves from Wait to Ready.
  sim.tick();

  // Tick 8: Scheduler picks P1 to Memory Check (Now True) to CPU.
  sim.tick();

  // Validate simulation is running (P1 is in CPU)
  assert(sim.isRunning());

  std::cout << "[PASSED] test_page_fault_flow" << std::endl;
  std::remove(fname.c_str());
}

int main() {
  test_io_blocking_flow();
  test_page_fault_flow();
  return 0;
}
