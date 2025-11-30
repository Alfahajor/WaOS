/**
 * @brief Abstract interface for CPU schedulers.
 *
 * This interface defines the contract that all scheduling algorithms
 * must implement so the Simulator Core can operate with any scheduler.
 */

#pragma once

#include <vector>
#include <memory>
#include <string>
#include "waos/common/DataStructures.h"

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
     * @brief Defines the maximum CPU ticks a process can run before preemption.
     * @return Positive integer for Quantum, or -1 for infinite (no timer preemption).
     */
    virtual int getTimeSlice() const = 0;

    /**
     * @brief Observa la cola de procesos listos SIN removerlos.
     * Crucial para que la GUI muestre la cola "Ready" sin afectar la simulación.
     */
    virtual std::vector<const waos::core::Process*> peekReadyQueue() const = 0;

    /**
     * @brief Obtiene el nombre del algoritmo (ej: "Round Robin", "FCFS").
     */
    virtual std::string getAlgorithmName() const = 0;

    /**
     * @brief Obtiene métricas internas del planificador.
     */
    virtual waos::common::SchedulerMetrics getSchedulerMetrics() const = 0;
  };

}
