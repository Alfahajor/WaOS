/**
 * @file PriorityScheduler.h
 * @brief Priority-based scheduler skeleton.
 *
 * For now, all processes are assigned a default priority of 0
 * (until Process is extended with an explicit priority).
 */

#pragma once

#include <deque>
#include <map>
#include <mutex>

#include "IScheduler.h"
#include "waos/common/DataStructures.h"

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
  PriorityScheduler();
  ~PriorityScheduler() override = default;

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
  std::map<int, std::deque<waos::core::Process*>> m_queues;  // < priority to queue
  waos::common::SchedulerMetrics m_metrics;
};

}  // namespace waos::scheduler
