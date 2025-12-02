#include "waos/scheduler/PriorityScheduler.h"

#include <iostream>

#include "waos/core/Process.h"

namespace waos::scheduler {

PriorityScheduler::PriorityScheduler() {
  m_metrics.totalSchedulingDecisions = 0;
}

void PriorityScheduler::addProcess(waos::core::Process* p) {
  if (!p) return;
  std::lock_guard<std::mutex> lock(m_mutex);

  // Currently Process has no priority getter
  int priority = p->getPriority();

  m_queues[priority].push_back(p);
  std::cout << "  [Priority] Added P" << p->getPid() << " (Prio " << priority << ") to ready queue" << std::endl;
}

waos::core::Process* PriorityScheduler::getNextProcess() {
  std::lock_guard<std::mutex> lock(m_mutex);

  // Iterate from highest priority (lowest key)
  for (auto it = m_queues.begin(); it != m_queues.end();) {
    auto& q = it->second;
    if (!q.empty()) {
      waos::core::Process* p = q.front();
      q.pop_front();

      // If queue becomes empty, we can remove the key from map to keep it clean
      if (q.empty()) {
        m_queues.erase(it);
      }

      m_metrics.totalSchedulingDecisions++;
      m_metrics.selectionCount[p->getPid()]++;

      std::cout << "  [Priority] Selected P" << p->getPid()
                << " (Prio " << p->getPriority() << ") for execution" << std::endl;
      return p;
    } else {
      // Empty queue in map, remove it and continue
      it = m_queues.erase(it);
    }
  }
  return nullptr;
}

bool PriorityScheduler::hasReadyProcesses() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  for (const auto& kv : m_queues) {
    if (!kv.second.empty()) return true;
  }
  return false;
}

int PriorityScheduler::getTimeSlice() const {
  return -1;  // Non-preemptive priority by default
}

std::vector<const waos::core::Process*> PriorityScheduler::peekReadyQueue() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<const waos::core::Process*> result;

  for (const auto& kv : m_queues) {
    const auto& q = kv.second;
    for (const auto* p : q) {
      result.push_back(p);
    }
  }
  return result;
}

std::string PriorityScheduler::getAlgorithmName() const {
  return "Priority Scheduling";
}

waos::common::SchedulerMetrics PriorityScheduler::getSchedulerMetrics() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_metrics;
}

}  // namespace waos::scheduler
