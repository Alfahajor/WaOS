/**
 * @file FCFSScheduler.h
 * @brief First-Come, First-Served scheduler header.
 */

#pragma once

#include "IScheduler.h"
#include <queue>
#include <mutex>
#include "waos/common/DataStructures.h"

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
    FCFSScheduler();
    ~FCFSScheduler() override = default;

    void addProcess(waos::core::Process* p) override;
    waos::core::Process* getNextProcess() override;
    bool hasReadyProcesses() const override;
    int getTimeSlice() const override;

    // Métodos de Observación
    std::vector<const waos::core::Process*> peekReadyQueue() const override;
    std::string getAlgorithmName() const override;
    waos::common::SchedulerMetrics getSchedulerMetrics() const override;

private:
    std::queue<waos::core::Process*> m_queue;
    mutable std::mutex m_mutex;
    waos::common::SchedulerMetrics m_metrics; // Métricas internas
};

}
