#pragma once

#include <QObject>
#include <memory>
#include <vector>

#include "MockMemoryManager.h"
#include "MockScheduler.h"
#include "waos/common/DataStructures.h"
#include "waos/core/Process.h"
#include "waos/core/Simulator.h"

namespace waos::gui::mock {

class MockSimulator : public QObject {
  Q_OBJECT
 public:
  explicit MockSimulator(QObject* parent = nullptr);
  ~MockSimulator() override = default;

  void start();
  void stop();
  void reset();
  void tick();

  // Getters required by GUI
  std::vector<const waos::core::Process*> getAllProcesses() const;
  const waos::core::Process* getRunningProcess() const;
  std::vector<const waos::core::Process*> getBlockedProcesses() const;
  std::vector<waos::core::MemoryWaitInfo> getMemoryWaitQueue() const;
  std::vector<const waos::core::Process*> getReadyProcesses() const;

  waos::common::SimulatorMetrics getSimulatorMetrics() const;
  std::string getSchedulerAlgorithmName() const;
  std::string getMemoryAlgorithmName() const;

  const MockScheduler* getScheduler() const { return m_scheduler.get(); }
  const MockMemoryManager* getMemoryManager() const { return m_memoryManager.get(); }

 signals:
  void clockTicked(uint64_t currentTime);
  void processStateChanged(int pid, waos::core::ProcessState newState);
  void simulationFinished();

 private:
  uint64_t m_currentTick = 0;
  bool m_isRunning = false;

  std::unique_ptr<MockScheduler> m_scheduler;
  std::unique_ptr<MockMemoryManager> m_memoryManager;

  std::vector<std::unique_ptr<waos::core::Process>> m_processes;
};

}  // namespace waos::gui::mock
