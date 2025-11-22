#pragma once
/**
 * @file FCFSScheduler.h
 * @brief First-Come, First-Served scheduler header.
 */

#include "IScheduler.h"
#include <queue>
#include <mutex>

namespace waos::core {
    class Process;
}

namespace waos::scheduler {

/**
 * @class FCFSScheduler
 * @brief Simple FIFO scheduler (first-come, first-served).
 *
 * getNextProcess() returns and removes the head of the queue.
 */
class FCFSScheduler : public IScheduler {
public:
    FCFSScheduler() = default;
    ~FCFSScheduler() override = default;

    void addProcess(waos::core::Process* p) override;
    waos::core::Process* getNextProcess() override;
    bool hasReadyProcesses() const override;

private:
    std::queue<waos::core::Process*> m_queue;
    mutable std::mutex m_mutex;
};

} // namespace waos::scheduler
