#pragma once
/**
 * @file RRScheduler.h
 * @brief Round-Robin scheduler skeleton.
 *
 * Real RR logic (time slicing and requeueing) will be implemented later.
 */

#include "IScheduler.h"
#include <queue>
#include <mutex>
#include <chrono>

namespace waos::core {
    class Process;
}

namespace waos::scheduler {

/**
 * @class RRScheduler
 * @brief Round Robin scheduler stub.
 *
 * The class stores processes in a queue and exposes a configurable quantum.
 */
class RRScheduler : public IScheduler {
public:
    explicit RRScheduler(std::chrono::milliseconds quantum = std::chrono::milliseconds(100))
        : m_quantum(quantum) {}

    ~RRScheduler() override = default;

    void addProcess(waos::core::Process* p) override;
    waos::core::Process* getNextProcess() override;
    bool hasReadyProcesses() const override;

    std::chrono::milliseconds getQuantum() const { return m_quantum; }

private:
    std::chrono::milliseconds m_quantum;
    mutable std::mutex m_mutex;
    std::queue<waos::core::Process*> m_queue;
};

} // namespace waos::scheduler
