/**
 * @file PriorityScheduler.h
 * @brief Priority-based scheduler skeleton.
 *
 * For now, all processes are assigned a default priority of 0
 * (until Process is extended with an explicit priority).
 */

#pragma once

#include "IScheduler.h"
#include <map>
#include <deque>
#include <mutex>

namespace waos::core {
    class Process;
}

namespace waos::scheduler {

/**
 * @class PriorityScheduler
 * @brief Priority scheduler stub (multiple queues per priority level).
 *
 * Lower integer value denotes higher priority (0 = highest).
 */
class PriorityScheduler : public IScheduler {
public:
    PriorityScheduler() = default;
    ~PriorityScheduler() override = default;

    void addProcess(waos::core::Process* p) override;
    waos::core::Process* getNextProcess() override;
    bool hasReadyProcesses() const override;
    int getTimeSlice() const override;

private:
    mutable std::mutex m_mutex;
    std::map<int, std::deque<waos::core::Process*>> m_queues; // < priority to queue
};

}
