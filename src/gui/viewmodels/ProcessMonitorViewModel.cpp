#include "ProcessMonitorViewModel.h"

#include <QDebug>

namespace waos::gui::viewmodels {

ProcessMonitorViewModel::ProcessMonitorViewModel(QObject* parent) : QObject(parent) {}

void ProcessMonitorViewModel::setSimulator(waos::core::Simulator* simulator) {
  m_simulator = simulator;
  if (m_simulator) {
    connect(m_simulator, &waos::core::Simulator::clockTicked,
            this, &ProcessMonitorViewModel::onClockTicked);
  }
}

void ProcessMonitorViewModel::onClockTicked(uint64_t tick) {
  if (!m_simulator) return;

  auto allProcesses = m_simulator->getAllProcesses();

  qDeleteAll(m_processItems);
  m_processItems.clear();

  for (const auto* proc : allProcesses) {
    auto* item = new waos::gui::models::ProcessItemModel(this);
    item->updateFrom(proc);
    m_processItems.append(item);
  }

  emit processListChanged();

  auto metrics = m_simulator->getSimulatorMetrics();
  if (m_avgWaitTime != metrics.avgWaitTime) {
    m_avgWaitTime = metrics.avgWaitTime;
    emit avgWaitTimeChanged();
  }
  if (m_avgTurnaroundTime != metrics.avgTurnaroundTime) {
    m_avgTurnaroundTime = metrics.avgTurnaroundTime;
    emit avgTurnaroundTimeChanged();
  }
  if (m_cpuUtilization != metrics.cpuUtilization) {
    m_cpuUtilization = metrics.cpuUtilization;
    emit cpuUtilizationChanged();
  }
}

void ProcessMonitorViewModel::reset() {
  qDeleteAll(m_processItems);
  m_processItems.clear();
  emit processListChanged();

  m_avgWaitTime = 0.0;
  emit avgWaitTimeChanged();

  m_avgTurnaroundTime = 0.0;
  emit avgTurnaroundTimeChanged();

  m_cpuUtilization = 0.0;
  emit cpuUtilizationChanged();
}

}  // namespace waos::gui::viewmodels
