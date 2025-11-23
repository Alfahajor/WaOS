/**
 * @brief Defines the main orchestration class for the OS simulation.
 * @version 0.2
 * @date 11-22-2025
 */

#pragma once

#include <QObject>
#include <memory>
#include <vector>
#include <mutex>
#include <list>

#include "waos/core/Process.h"
#include "waos/core/Clock.h"
#include "waos/scheduler/IScheduler.h"
#include "waos/memory/IMemoryManager.h"

namespace waos::core {

  // Tracks a process waiting for a page to be loaded (Page Fault penalty).
  struct MemoryWaitInfo {
    Process* process;
    int ticksRemaining; // Penalización restante
  };

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

    /**
     * @brief Starts the simulation loop. This might run in a separate thread
     */
    void start();

    /**
     * @brief Stops or pauses the simulation.
     */
    void stop();

    /**
     * @brief Resets the simulation to its initial state (time 0).
     */
    void reset();

    // public to facilitate step-by-step unit testing
    void tick();

    // Getters for validation in tests
    uint64_t getCurrentTime() const;
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

    // The Simulator owns all processes.
    std::vector<std::unique_ptr<Process>> m_processes;
    
    // Separate container for processes that haven't arrived yet (Waiting to arrive)
    std::vector<Process*> m_incomingProcesses;

    // Queue for processes blocked by I/O (The simulator manages I/O waits)
    std::vector<Process*> m_blockedQueue; 

    // Processes waiting for the disk to load a page
    std::list<MemoryWaitInfo> m_memoryWaitQueue; 

    // Process currently in CPU
    Process* m_runningProcess;
    bool m_isRunning;
    mutable std::mutex m_simulationMutex; // For thread safety in future steps

    // Penalty configuration
    const int PAGE_FAULT_PENALTY = 5; 

    /**
     * @brief The main logic step executed every tick.
     */
    void step();

    // Helpers
    void handleArrivals();
    void handleIO();
    void handlePageFaults();
    void handleCpuExecution();
    void handleScheduling();
  };

}
