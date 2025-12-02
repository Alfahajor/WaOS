#include "waos/scheduler/RRScheduler.h"

#include <iostream>

#include "waos/core/Process.h"

namespace waos::scheduler {

RRScheduler::RRScheduler(int quantum) : m_quantum(quantum) {
  if (m_quantum <= 0) m_quantum = 4;  // Fallback seguro
  m_metrics.totalSchedulingDecisions = 0;
}

void RRScheduler::addProcess(waos::core::Process* p) {
  if (!p) return;
  std::lock_guard<std::mutex> lock(m_mutex);
  m_queue.push(p);
  std::cout << "  [RR] Added P" << p->getPid() << " to ready queue (FIFO order)" << std::endl;
}

waos::core::Process* RRScheduler::getNextProcess() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_queue.empty()) return nullptr;

  waos::core::Process* p = m_queue.front();
  m_queue.pop();

  // Update metrics
  m_metrics.totalSchedulingDecisions++;
  m_metrics.selectionCount[p->getPid()]++;

  std::cout << "  [RR] Selected P" << p->getPid() << " for execution (Quantum="
            << m_quantum << " ticks)" << std::endl;

  return p;
}

bool RRScheduler::hasReadyProcesses() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return !m_queue.empty();
}

int RRScheduler::getTimeSlice() const {
  return m_quantum;
}

std::vector<const waos::core::Process*> RRScheduler::peekReadyQueue() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<const waos::core::Process*> result;

  std::queue<waos::core::Process*> temp = m_queue;
  while (!temp.empty()) {
    result.push_back(temp.front());
    temp.pop();
  }
  return result;
}

std::string RRScheduler::getAlgorithmName() const {
  return "Round Robin (Q=" + std::to_string(m_quantum) + ")";
}

waos::common::SchedulerMetrics RRScheduler::getSchedulerMetrics() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_metrics;
}

}  // namespace waos::scheduler
