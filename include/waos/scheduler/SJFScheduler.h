/**
 * @file SJFScheduler.h
 * @brief Shortest Job First scheduler using priority queue.
 *
 * Implements SJF algorithm using std::priority_queue for O(log n) insertions.
 */

#pragma once

#include "IScheduler.h"
#include <vector>
#include <queue>
#include <mutex>
#include "waos/common/DataStructures.h"

namespace waos::core {
    class Process;
}

namespace waos::scheduler {

/**
 * @class SJFScheduler
 * @brief Shortest Job First scheduler using priority queue.
 *
 * Uses std::priority_queue with custom comparator to maintain processes
 * ordered by current CPU burst duration. Provides O(log n) insertion
 * and O(log n) extraction complexity.
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
    /**
     * @brief Comparator for priority queue (min-heap by burst duration)
     */
    struct ProcessComparator {
        bool operator()(waos::core::Process* a, waos::core::Process* b) const;
    };

    mutable std::mutex m_mutex;
    std::priority_queue<waos::core::Process*, 
                        std::vector<waos::core::Process*>,
                        ProcessComparator> m_priorityQueue;
    waos::common::SchedulerMetrics m_metrics;
};

}
