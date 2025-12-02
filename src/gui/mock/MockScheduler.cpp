#include "MockScheduler.h"

#include "MockDataProvider.h"

namespace waos::gui::mock {

MockScheduler::MockScheduler() {}

void MockScheduler::addProcess(waos::core::Process* p) {
  if (p) m_readyQueue.push_back(p);
}

waos::core::Process* MockScheduler::getNextProcess() {
  if (m_readyQueue.empty()) return nullptr;
  auto* p = m_readyQueue.front();
  m_readyQueue.pop_front();
  return p;
}

bool MockScheduler::hasReadyProcesses() const {
  return !m_readyQueue.empty();
}

int MockScheduler::getTimeSlice() const {
  return m_timeSlice;
}

std::vector<const waos::core::Process*> MockScheduler::peekReadyQueue() const {
  std::vector<const waos::core::Process*> result;
  for (const auto* p : m_readyQueue) {
    result.push_back(p);
  }
  return result;
}

size_t MockScheduler::getReadyQueueSize() const {
  return m_readyQueue.size();
}

std::string MockScheduler::getAlgorithmName() const {
  return "Mock Scheduler (Round Robin)";
}

waos::common::SchedulerMetrics MockScheduler::getSchedulerMetrics() const {
  return MockDataProvider::generateSchedulerMetrics(0);
}

}  // namespace waos::gui::mock
