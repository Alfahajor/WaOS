#include "ProcessMonitorViewModel.h"

#include <QDebug>

namespace waos::gui::viewmodels {

ProcessMonitorViewModel::ProcessMonitorViewModel(QObject* parent) : QObject(parent) {}

void ProcessMonitorViewModel::setSimulator(waos::gui::mock::MockSimulator* simulator) {
  m_simulator = simulator;
  if (m_simulator) {
    connect(m_simulator, &waos::gui::mock::MockSimulator::clockTicked,
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

QString ProcessMonitorViewModel::processStateToString(waos::core::ProcessState state) const {
  switch (state) {
    case waos::core::ProcessState::NEW:
      return "Nuevo";
    case waos::core::ProcessState::READY:
      return "Listo";
    case waos::core::ProcessState::RUNNING:
      return "Ejecutando";
    case waos::core::ProcessState::BLOCKED:
      return "Bloqueado (E/S)";
    case waos::core::ProcessState::WAITING_MEMORY:
      return "Bloqueado (Memoria)";
    case waos::core::ProcessState::TERMINATED:
      return "Terminado";
    default:
      return "Desconocido";
  }
}

}  // namespace waos::gui::viewmodels
