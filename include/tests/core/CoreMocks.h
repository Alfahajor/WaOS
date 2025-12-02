/**
 * @brief Mocks compartidos para IScheduler e IMemoryManager que cumplen con la interfaz completa.
 */

#pragma once

#include "waos/scheduler/IScheduler.h"
#include "waos/memory/IMemoryManager.h"
#include "waos/core/Process.h"
#include <list>
#include <map>
#include <vector>
#include <iostream>

using namespace waos::core;
using namespace waos::scheduler;
using namespace waos::memory;
using namespace waos::common;

class MockScheduler : public IScheduler {
public:
  std::list<Process*> readyQueue;
  int timeSlice = -1; // -1 = No quantum by default

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

  int getTimeSlice() const override { return timeSlice; }

  // Métodos de visualización (mockeados para cumplir interfaz)
  std::vector<const Process*> peekReadyQueue() const override {
    std::vector<const Process*> ret;
    for(auto* p : readyQueue) ret.push_back(p);
    return ret;
  }

  std::string getAlgorithmName() const override { return "MockScheduler"; }

  SchedulerMetrics getSchedulerMetrics() const override {
    return SchedulerMetrics();
  }
};

class MockMemoryManager : public IMemoryManager {
public:
  // Map <PID, <PageNumber, IsLoaded>>
  std::map<int, std::map<int, bool>> memoryState;

  // Para tracking de tests
  int requestCount = 0;

  // Flag to simulate infinite/perfect memory
  bool everythingLoaded = false;

  bool isPageLoaded(int pid, int page) const override {
    if (everythingLoaded) return true;

    if (memoryState.count(pid) && memoryState.at(pid).count(page)) {
      return memoryState.at(pid).at(page);
    }
    return false;
  }

  PageRequestResult requestPage(int pid, int page) override {
    requestCount++;
    // En simulación real, esto iniciaría reemplazo.
    // Aquí simulamos fallo siempre a menos que se fuerce carga externa.
    return PageRequestResult::PAGE_FAULT;
  }

  void completePageLoad(int pid, int pageNumber) override {
    memoryState[pid][pageNumber] = true;
  }

  void allocateForProcess(int pid, int requiredPages) override {
    // Inicializar mapa para evitar accesos inválidos
    for(int i=0; i<requiredPages; i++) memoryState[pid][i] = false;
  }

  void freeForProcess(int pid) override {
    memoryState.erase(pid);
  }

  // Métodos de visualización (mockeados para cumplir interfaz)
  std::vector<FrameInfo> getFrameStatus() const override { return {}; }
  std::vector<PageTableEntryInfo> getPageTableForProcess(int) const override { return {}; }

  MemoryStats getMemoryStats() const override {
    MemoryStats stats;
    stats.totalPageFaults = requestCount;
    return stats;
  }

  std::string getAlgorithmName() const override { return "MockMemory"; }
};
