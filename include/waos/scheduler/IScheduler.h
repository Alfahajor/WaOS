/**
 * @brief Abstract interface for CPU schedulers.
 *
 * This interface defines the contract that all scheduling algorithms
 * must implement so the Simulator Core can operate with any scheduler.
 */

#pragma once

#include <vector>
#include <memory>

namespace waos::core {
  class Process; // forward declaration
}

namespace waos::scheduler {

  /**
   * @class IScheduler
   * @brief Abstract base interface for process schedulers.
   *
   * Implementations must be thread-safe if used from multiple threads.
   * The convention here is that getNextProcess() returns and removes the
   * chosen process from the scheduler's internal structure (i.e., it consumes it).
   */
  class IScheduler {
  public:
    virtual ~IScheduler() = default;

    /**
     * @brief Enqueue a process into the scheduler.
     *
     * The scheduler does not take ownership of the pointer.
     * Caller is responsible for process lifetime management.
     *
     * @param p Pointer to waos::core::Process (must be valid).
     */
    virtual void addProcess(waos::core::Process* p) = 0;
    virtual void addProcess(std::unique_ptr<waos::core::Process> p) = 0;

    /**
     * @brief Select, remove and return the next process according to the policy.
     *
     * @return Pointer to the selected Process, or nullptr when none available.
     */
    virtual waos::core::Process* getNextProcess() = 0;

    /**
     * @brief Returns true when scheduler has one or more ready processes.
     *
     * @return true if ready processes exist, false otherwise.
     */
    virtual bool hasReadyProcesses() const = 0;

    /**
     * @brief Informs the scheduler that a time unit (tick) has passed.
     *
     * Some algorithms (like aging) may need this to update priorities.
     */
    virtual void tick() = 0;
  };

}
