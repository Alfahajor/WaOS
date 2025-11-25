/**
 * @file ProcessItemModel.cpp
 * @brief Implementación del wrapper de Process para QML
 */

#include "models/ProcessItemModel.h"

namespace waos::gui::models {

ProcessItemModel::ProcessItemModel(QObject* parent)
    : QObject(parent) {
}

void ProcessItemModel::updateFrom(const waos::core::Process* process) {
  if (!process) return;

  // Actualizar datos básicos
  m_pid = process->getPid();
  m_priority = process->getPriority();
  m_arrivalTime = process->getArrivalTime();
  m_requiredPages = process->getRequiredPages();

  // Actualizar estado
  QString newState = stateToString(process->getState());
  if (m_state != newState) {
    m_state = newState;
    emit stateChanged();
  }

  // Actualizar estadísticas
  const auto& stats = process->getStats();
  bool statsUpdated = false;

  if (m_waitTime != stats.totalWaitTime) {
    m_waitTime = stats.totalWaitTime;
    statsUpdated = true;
  }

  if (m_cpuTime != stats.totalCpuTime) {
    m_cpuTime = stats.totalCpuTime;
    statsUpdated = true;
  }

  if (m_ioTime != stats.totalIoTime) {
    m_ioTime = stats.totalIoTime;
    statsUpdated = true;
  }

  if (m_pageFaults != stats.pageFaults) {
    m_pageFaults = stats.pageFaults;
    statsUpdated = true;
  }

  if (m_preemptions != stats.preemptions) {
    m_preemptions = stats.preemptions;
    statsUpdated = true;
  }

  if (statsUpdated) {
    emit statsChanged();
  }

  // Actualizar burst actual
  QString newBurst;
  if (process->hasMoreBursts()) {
    auto type = process->getCurrentBurstType();
    int duration = process->getCurrentBurstDuration();
    newBurst = (type == waos::core::BurstType::CPU ? "CPU" : "E/S") +
               QString("(%1)").arg(duration);
  } else {
    newBurst = "Completado";
  }

  if (m_currentBurst != newBurst) {
    m_currentBurst = newBurst;
    emit burstChanged();
  }
}

QString ProcessItemModel::stateToString(waos::core::ProcessState state) const {
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

}  // namespace waos::gui::models
