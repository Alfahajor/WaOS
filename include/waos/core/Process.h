/**
 * @brief Defines the Process Control Block (PCB) data structure for the simulator.
 * @version 0.1
 * @date 11-11-2025
 */

#pragma once

#include <cstdint>
#include <vector>
#include <queue>

namespace waos::core {

  /**
   * @enum ProcessState
   * @brief Represents the possible states of a process during its lifecycle.
   */
  enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
  };

  /**
   * @struct ProcessStats
   * @brief A container for collecting performance metrics for a single process.
   */
  struct ProcessStats {
    uint64_t startTime = 0;
    uint64_t finishTime = 0;     
    uint64_t totalWaitTime = 0;
    uint64_t totalCpuTime = 0;
    uint64_t lastReadyTime = 0;
  };

  /**
   * @class Process
   * @brief Represents a process and its associated Process Control Block (PCB).
   *
   * This class is primarily a data container. It holds all static and dynamic
   * information about a process, such as its ID, state, scheduling parameters,
   * memory requirements, and performance statistics.
   */
  class Process {
  public:
    /**
     * @brief Constructs a new Process instance.
     * @param pid The unique process identifier.
     * @param arrivalTime The simulation time at which the process arrives.
     * @param cpuBursts A queue of CPU burst durations.
     * @param requiredPages The number of memory pages this process requires.
     */
    Process(int pid, uint64_t arrivalTime, std::queue<int> cpuBursts, int requiredPages);

    int getPid() const;
    uint64_t getArrivalTime() const;
    int getRequiredPages() const;

    ProcessState getState() const;
    void setState(ProcessState newState, uint64_t currentTime);

    int getCurrentCpuBurst() const;
    void advanceToNextBurst();
    bool hasMoreBursts() const;

    const ProcessStats& getStats() const;
    void addCpuTime(uint64_t time);

  private:
    int m_pid;
    ProcessState m_state;

    // Scheduling Information
    uint64_t m_arrivalTime;
    std::queue<int> m_cpuBursts;
    
    // Memory Information
    int m_requiredPages;

    ProcessStats m_stats;
  };

}
