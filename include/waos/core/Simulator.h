/**
 * @brief Defines the main orchestration class for the OS simulation.
 * @version 0.4
 * @date 11-22-2025
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

/**
 * @class Simulator
 * @brief The central engine of the operating system simulator.
 *
 * This class orchestrates the interaction between the Scheduler, Memory Manager,
 * and the simulation Clock. It manages the lifecycle of processes and exposes
 * signals to update the User Interface.
 */
class Simulator : public QObject {
  Q_OBJECT

 public:
  explicit Simulator(QObject* parent = nullptr);
  ~Simulator() override;

  /**
   * @brief Loads processes from a configuration file using the Parser.
   * @param filePath Path to the .txt file containing process definitions.
   * @return True if loading was successful, false otherwise.
   */
  bool loadProcesses(const std::string& filePath);

  /**
   * @brief Injects the specific scheduling algorithm to be used.
   * @param scheduler Ownership of a concrete IScheduler implementation.
   */
  void setScheduler(std::unique_ptr<waos::scheduler::IScheduler> scheduler);

  /**
   * @brief Injects the specific memory management strategy to be used.
   * @param memoryManager Ownership of a concrete IMemoryManager implementation.
   */
  void setMemoryManager(std::unique_ptr<waos::memory::IMemoryManager> memoryManager);

  // Simulation
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

  waos::common::SimulatorMetrics getSimulatorMetrics() const;
  std::string getSchedulerAlgorithmName() const;
  std::string getMemoryAlgorithmName() const;

  // Accessors for sub-components (Read-Only)
  const waos::scheduler::IScheduler* getScheduler() const;
  const waos::memory::IMemoryManager* getMemoryManager() const;

  // Memory Wrappers to prevent Deadlocks (SimulatorMutex -> MemoryMutex order)
  std::vector<waos::common::FrameInfo> getFrameStatus() const;
  std::vector<waos::common::PageTableEntryInfo> getPageTableForProcess(int processId) const;
  waos::common::MemoryStats getMemoryStats() const;

  // Getters for validation in tests
  uint64_t getCurrentTime() const;
  const uint64_t* getClockRef() const;
  bool isRunning() const;

 signals:

  /**
   * @brief Emitted when the global simulation clock ticks.
   * @param currentTime The new time value.
   */
  void clockTicked(uint64_t currentTime);

  /**
   * @brief Emitted when a process changes its state
   * @param pid Process Identifier.
   * @param newState The new state of the process.
   */
  void processStateChanged(int pid, ProcessState newState);

  /**
   * @brief Emitted when the simulation finishes (all processes terminated).
   */
  void simulationFinished();

  /**
   * @brief Emitted to log messages to the UI console.
   * @param message The log string.
   */
  void logMessage(QString message);

 private:
  Clock m_clock;
  std::unique_ptr<waos::scheduler::IScheduler> m_scheduler;
  std::unique_ptr<waos::memory::IMemoryManager> m_memoryManager;

  SystemMonitor m_systemMonitor;

  // The Simulator owns all processes.
  std::vector<std::unique_ptr<Process>> m_processes;

  // Separate container for processes that haven't arrived yet (Waiting to arrive)
  std::vector<Process*> m_incomingProcesses;

  // Queue for processes blocked by I/O (The simulator manages I/O waits)
  std::vector<Process*> m_blockedQueue;

  // Processes waiting for the disk to load a page
  struct InternalMemoryWait {
    Process* process;
    int ticksRemaining;
    int pageNumber;
  };
  std::list<InternalMemoryWait> m_memoryWaitQueue;

  // Process currently in CPU
  Process* m_runningProcess;

  // Context Switch Handling
  Process* m_nextProcess;
  int m_contextSwitchCounter;  // Ticks remaining for CS

  // Global Accumulators for Metrics
  uint64_t m_cpuActiveTicks;
  int m_totalPageFaults;
  int m_totalContextSwitches;
  waos::common::SimulatorMetrics m_metrics;

  bool m_isRunning;
  mutable std::recursive_mutex m_simulationMutex;  // Recursive to allow signal-slot re-entry

  int m_pageFaultPenalty;
  int m_contextSwitchDuration;

  // The main logic step executed every tick.
  void step();

  // Helpers to simulation
  void handleArrivals();
  void handleIO();
  void handlePageFaults();
  void handleCpuExecution();
  void handleScheduling();

  // Helper to initiate context switch
  void triggerContextSwitch(Process* current, Process* next);

  // Internal helper to refresh metric struct
  void updateMetrics();

  // Returns true if I/O burst finished in this step
  bool processIoStep(Process* p);
};

}  // namespace waos::core
