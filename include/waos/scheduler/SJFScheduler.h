#pragma once
/**
 * @file SJFScheduler.h
 * @brief Shortest Job First scheduler (skeleton).
 *
 * This is a skeleton: later we will sort by current CPU burst length.
 */

#include "IScheduler.h"
#include <vector>
#include <mutex>

namespace waos::core {
    class Process;
}

namespace waos::scheduler {

/**
 * @class SJFScheduler
 * @brief Shortest Job First scheduler skeleton.
 *
 * Currently acts as a compilable stub that stores processes in a vector.
 * Future implementation: select process with shortest next CPU burst.
 */
class SJFScheduler : public IScheduler {
public:
    SJFScheduler() = default;
    ~SJFScheduler() override = default;

    void addProcess(waos::core::Process* p) override;
    waos::core::Process* getNextProcess() override;
    bool hasReadyProcesses() const override;

private:
    mutable std::mutex m_mutex;
    std::vector<waos::core::Process*> m_pool; ///< unsorted pool (stub)
};

} // namespace waos::scheduler
