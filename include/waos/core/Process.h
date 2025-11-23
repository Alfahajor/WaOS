/**
 * @brief Defines the Process Control Block (PCB) data structure for the simulator.
 * @version 0.2
 * @date 11-21-2025
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
    WAITING_MEMORY, // Only to GUI color
    TERMINATED
  };

  /**
   * @enum BurstType
   * @brief Distinguishes between CPU processing and I/O waiting.
   */
  enum class BurstType {
    CPU,
    IO
  };

  /**
   * @struct Burst
   * @brief Represents a single unit of work or wait.
   */
  struct Burst {
    BurstType type;
    int duration;
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
    uint64_t totalIoTime = 0;
    uint64_t lastReadyTime = 0;
    int pageFaults = 0;
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
    Process(int pid, uint64_t arrivalTime, std::queue<Burst> bursts, int requiredPages);

    int getPid() const;
    uint64_t getArrivalTime() const;
    int getRequiredPages() const;

    ProcessState getState() const;
    void setState(ProcessState newState, uint64_t currentTime);

    /**
     * @brief Gets the type of the current burst (CPU or IO).
     */
    BurstType getCurrentBurstType() const;

    /**
     * @brief Gets the remaining duration of the current burst.
     */
    int getCurrentBurstDuration() const;

    /**
     * @brief Consumes time from the current burst.
     * @param timeUnits Amount of time to process.
     * @return True if the burst is completed, false otherwise.
     */
    bool updateCurrentBurst(int timeUnits);

    /**
     * @brief Removes the current finished burst and moves to the next.
     */
    void advanceToNextBurst();
    bool hasMoreBursts() const;

    /**
     * @brief Gets the page number the process needs to access in the current CPU tick.
     * @return The virtual page number (0 to requiredPages - 1).
     */
    int getCurrentPageRequirement() const;

    /**
     * @brief Advances the instruction pointer to the next memory reference.
     * Should be called after a successful CPU tick execution.
     */
    void advanceInstructionPointer();

    const ProcessStats& getStats() const;
    void addCpuTime(uint64_t time);
    void addIoTime(uint64_t time);
    void incrementPageFaults();

  private:
    int m_pid;
    ProcessState m_state;

    // Scheduling Information
    uint64_t m_arrivalTime;
    std::queue<Burst> m_bursts;
    
    // Memory Information
    int m_requiredPages;

    ProcessStats m_stats;

    // Memory Simulation Internal Data
    std::vector<int> m_pageReferenceString;
    size_t m_instructionPointer;

    // Generates a deterministic sequence of page references based on locality.
    void generateReferenceString();
  };

}
