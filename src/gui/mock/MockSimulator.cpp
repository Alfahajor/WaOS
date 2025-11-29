#include "MockSimulator.h"

#include "MockDataProvider.h"

namespace waos::gui::mock {

MockSimulator::MockSimulator(QObject* parent) : QObject(parent) {
  m_scheduler = std::make_unique<MockScheduler>();
  m_memoryManager = std::make_unique<MockMemoryManager>();
  m_processes = MockDataProvider::generateProcessList(0);
}

void MockSimulator::start() { m_isRunning = true; }
void MockSimulator::stop() { m_isRunning = false; }
void MockSimulator::reset() {
  m_currentTick = 0;
  m_processes = MockDataProvider::generateProcessList(0);
  emit clockTicked(m_currentTick);
}

void MockSimulator::tick() {
    // if (!m_isRunning) return; // Allow manual ticking
    
    m_currentTick++;
    m_processes = MockDataProvider::generateProcessList(m_currentTick);
    
    emit clockTicked(m_currentTick);
}std::vector<const waos::core::Process*> MockSimulator::getAllProcesses() const {
  std::vector<const waos::core::Process*> result;
  for (const auto& p : m_processes) {
    result.push_back(p.get());
  }
  return result;
}

const waos::core::Process* MockSimulator::getRunningProcess() const {
  for (const auto& p : m_processes) {
    if (p->getState() == waos::core::ProcessState::RUNNING) {
      return p.get();
    }
  }
  return nullptr;
}

std::vector<const waos::core::Process*> MockSimulator::getBlockedProcesses() const {
  std::vector<const waos::core::Process*> result;
  for (const auto& p : m_processes) {
    if (p->getState() == waos::core::ProcessState::BLOCKED) {
      result.push_back(p.get());
    }
  }
  return result;
}

std::vector<waos::core::MemoryWaitInfo> MockSimulator::getMemoryWaitQueue() const {
  std::vector<waos::core::MemoryWaitInfo> result;
  for (const auto& p : m_processes) {
    if (p->getState() == waos::core::ProcessState::WAITING_MEMORY) {
      result.push_back({p.get(), 5, 0});
    }
  }
  return result;
}

std::vector<const waos::core::Process*> MockSimulator::getReadyProcesses() const {
  std::vector<const waos::core::Process*> result;
  for (const auto& p : m_processes) {
    if (p->getState() == waos::core::ProcessState::READY) {
      result.push_back(p.get());
    }
  }
  return result;
}

waos::common::SimulatorMetrics MockSimulator::getSimulatorMetrics() const {
  return MockDataProvider::generateSimulatorMetrics(m_currentTick);
}

std::string MockSimulator::getSchedulerAlgorithmName() const {
  return m_scheduler->getAlgorithmName();
}

std::string MockSimulator::getMemoryAlgorithmName() const {
  return m_memoryManager->getAlgorithmName();
}

}  // namespace waos::gui::mock
