/**
 * @file SJFScheduler.h
 * @brief Shortest Job First scheduler (skeleton).
 *
 * This is a skeleton: later we will sort by current CPU burst length.
 */

#pragma once

#include "IScheduler.h"
#include <vector>
#include <mutex>
#include "waos/common/DataStructures.h"

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
    SJFScheduler();
    ~SJFScheduler() override = default;

    void addProcess(waos::core::Process* p) override;
    waos::core::Process* getNextProcess() override;
    bool hasReadyProcesses() const override;
    int getTimeSlice() const override;

    // Métodos de Observación
    std::vector<const waos::core::Process*> peekReadyQueue() const override;
    std::string getAlgorithmName() const override;
    waos::common::SchedulerMetrics getSchedulerMetrics() const override;
private:
    mutable std::mutex m_mutex;
    std::vector<waos::core::Process*> m_pool; // < unsorted pool
    waos::common::SchedulerMetrics m_metrics;
};

}
