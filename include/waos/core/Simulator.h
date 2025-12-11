/**
 * @brief Defines the main orchestration class for the OS simulation.
 * Refactored for deterministic tick execution.
 */

#pragma once

#include <QObject>
#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include "waos/common/DataStructures.h"
#include "waos/core/Clock.h"
#include "waos/core/Process.h"
#include "waos/core/SystemMonitor.h"
#include "waos/memory/IMemoryManager.h"
#include "waos/scheduler/IScheduler.h"

namespace waos::core {

class Simulator : public QObject {
  Q_OBJECT

public:
  explicit Simulator(QObject* parent = nullptr);
  ~Simulator() override;

  bool loadProcesses(const std::string& filePath);
  void setScheduler(std::unique_ptr<waos::scheduler::IScheduler> scheduler);
  void setMemoryManager(std::unique_ptr<waos::memory::IMemoryManager> memoryManager);

  void start();
  void stop();
  void reset();

  /**
   * @brief Executes one simulation tick.
   * @param force If true, executes even if m_isRunning is false (for manual stepping).
   */
  void tick(bool force = false);

  // Thread-safe getters
  std::vector<const Process*> getAllProcesses() const;
  const Process* getRunningProcess() const;
  std::vector<const Process*> getBlockedProcesses() const;
  std::vector<waos::common::MemoryWaitInfo> getMemoryWaitQueue() const;
  std::vector<const Process*> getReadyProcesses() const;

  // Component accessors
  waos::common::SimulatorMetrics getSimulatorMetrics() const;
  const waos::scheduler::IScheduler* getScheduler() const;
  const waos::memory::IMemoryManager* getMemoryManager() const;
  
  std::string getSchedulerAlgorithmName() const;
  std::string getMemoryAlgorithmName() const;

  // Memory specific getters
  std::vector<waos::common::FrameInfo> getFrameStatus() const;
  std::vector<waos::common::PageTableEntryInfo> getPageTableForProcess(int processId) const;
  waos::common::MemoryStats getMemoryStats() const;

  // Getters for validation in tests
  uint64_t getCurrentTime() const;
  const uint64_t* getClockRef() const;
  bool isRunning() const;
  bool isContextSwitching() const;

signals:
  void clockTicked(uint64_t currentTime);
  void processStateChanged(int pid, ProcessState newState);
  void simulationFinished();
  void logMessage(QString message);

public:
  enum class LogCategory { SYS, MEM, WAIT, NOTIFY, SCHED, PROC };

private:
  Clock m_clock;
  std::unique_ptr<waos::scheduler::IScheduler> m_scheduler;
  std::unique_ptr<waos::memory::IMemoryManager> m_memoryManager;
  SystemMonitor m_systemMonitor;

  std::vector<std::unique_ptr<Process>> m_processes;
  std::vector<Process*> m_incomingProcesses;
  std::list<Process*> m_blockedQueue;
  struct InternalMemoryWait {
    Process* process;
    int ticksRemaining;
    int pageNumber;
  };
  std::list<InternalMemoryWait> m_memoryWaitQueue;

  // Kernel states
  Process* m_runningProcess;
  Process* m_nextProcess; // Proceso seleccionado para entrar tras el CS

  int m_contextSwitchCounter; 
  const int m_contextSwitchDuration = 1;
  bool m_isKernelBusy;

  // Metrics
  uint64_t m_cpuActiveTicks;
  int m_totalPageFaults;
  int m_totalContextSwitches;
  waos::common::SimulatorMetrics m_metrics;
  const int m_pageFaultPenalty = 5;

  bool m_isRunning;
  mutable std::recursive_mutex m_simulationMutex;

  // Core logical
  void step();

  // Tick phases (Return true if a higher priority process became READY)
  bool handleArrivals();
  bool handleIO();
  bool handleMemoryOps();

  // Kernel management
  void executeKernelCycle();
  void executeUserCycle();
  
  // Context Transitions 
  void triggerContextSwitch(Process* current, ProcessState nextState, Process* next = nullptr);
  void completeContextSwitch();

  // Checks
  bool checkPreemption();
  void updateMetrics();
  void log(const QString& message, LogCategory category = LogCategory::SYS);
};

}
