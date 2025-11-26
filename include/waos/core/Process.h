/**
 * @brief Defines the Process Control Block (PCB) data structure for the simulator.
 * @version 0.4
 * @date 11-25-2025
 */

#pragma once

#include <cstdint>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

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
    int preemptions = 0; // Statistic for context switches forced by OS
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
     * @param priority The priority value
     * @param cpuBursts A queue of CPU burst durations.
     * @param requiredPages The number of memory pages this process requires.
     */
    Process(int pid, uint64_t arrivalTime, int priority, std::queue<Burst> bursts, int requiredPages);
    ~Process(); // Destructor to join thread

    /**
     * @brief Starts the internal OS thread.
     * Should be called when the process is created or arrives.
     */
    void startThread();

    /**
     * @brief Signals the process thread to execute one CPU tick.
     * Unblocks the internal run loop.
     */
    void signalRun();

    /**
     * @brief Signals the process thread to stop and join.
     * Used for cleanup or forced termination.
     */
    void stopThread();

    /**
     * @brief Blocks the caller (Kernel) until the process finishes its current tick logic.
     * Ensures synchronization before advancing the global clock.
     */
    void waitForTickCompletion();

    int getPid() const;
    uint64_t getArrivalTime() const;
    int getRequiredPages() const;
    int getPriority() const; // Lower value = Higher priority

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
     * @brief Gets the complete page reference string for this process.
     * @return Const reference to the vector of page references.
     */
    const std::vector<int>& getPageReferenceString() const;

    // Quantum Management
    int getQuantumUsed() const;
    void resetQuantum();
    void incrementQuantum(int ticks); // Used by Kernel to track preemption

    const ProcessStats& getStats() const;

    // Stat updaters (Thread-Safe or called by Kernel)
    void addCpuTime(uint64_t time);
    void addIoTime(uint64_t time);
    void incrementPageFaults();
    void incrementPreemptions();

  private:
    int m_pid;
    int m_priority;
    std::atomic<ProcessState> m_state; // Atomic for thread safety

    uint64_t m_arrivalTime;

    // Protected by m_processMutex
    std::queue<Burst> m_bursts;

    int m_quantumUsed;
    int m_requiredPages;
    ProcessStats m_stats;

    // Memory Simulation Internal Data
    std::vector<int> m_pageReferenceString;
    size_t m_instructionPointer;

    // Threading Infrastructure
    std::thread m_thread;
    mutable std::mutex m_processMutex;     // Protects shared data
    std::condition_variable m_cvRun;       // Wait for Kernel signal (Dispatch)
    std::condition_variable m_cvKernel;    // Notify Kernel (Yield/Done)

    bool m_running;             // Predicate for m_cvRun (Is it my turn?)
    bool m_tickCompleted;       // Predicate for m_cvKernel (Did I finish?)
    std::atomic<bool> m_stopThread; // Signal to exit thread loop

    /**
     * @brief Main loop executed by the internal thread.
     * Simulates the Fetch-Decode-Execute cycle.
     */
    void run();

    /**
     * @brief Internal logic to execute one burst unit.
     * @return True if the burst finished in this tick.
     */
    bool executeOneTick();

    // Generates a deterministic sequence of page references based on locality.
    void generateReferenceString();
    void advanceInstructionPointer();
  };

}
