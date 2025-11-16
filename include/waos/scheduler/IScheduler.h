#ifndef WAOS_SCHEDULER_ISCHEDULER_H
#define WAOS_SCHEDULER_ISCHEDULER_H

namespace waos {
namespace core {
    class Process;  // forward declaration
}

/**
 * @brief Base abstract interface for all CPU schedulers.
 *
 * All scheduling strategies (FCFS, SJF, RR, Priority) must derive
 * from this interface so the Simulator Core can swap them without
 * depending on specific implementations.
 */
class IScheduler {
public:
    virtual ~IScheduler() = default;

    /**
     * @brief Add a process to the scheduler's internal structure.
     * Note: The scheduler does NOT take ownership of the pointer.
     * @param p pointer to a valid Process object.
     */
    virtual void addProcess(waos::core::Process* p) = 0;

    /**
     * @brief Retrieve and remove the next ready process.
     * @return pointer to the selected process, or nullptr if none are available.
     */
    virtual waos::core::Process* getNextProcess() = 0;

    /**
     * @brief Check if the scheduler has ready processes.
     * @return true if ready processes exist, false otherwise.
     */
    virtual bool hasReadyProcesses() const = 0;
};

} // namespace waos

#endif // WAOS_SCHEDULER_ISCHEDULER_H