/**
 * @file ProcessItemModel.h
 * @brief Wrapper Qt para exponer datos de Process a QML
 */

#pragma once

#include <QObject>
#include <QString>

#include "waos/core/Process.h"

namespace waos::gui::models {

/**
 * @class ProcessItemModel
 * @brief Wrapper QObject para un Process, exponible a QML
 *
 * Encapsula los datos de un Process en properties Qt que pueden
 * ser consumidas por delegados QML en ListView/TableView.
 */
class ProcessItemModel : public QObject {
  Q_OBJECT

  Q_PROPERTY(int pid READ pid CONSTANT)
  Q_PROPERTY(QString state READ state NOTIFY stateChanged)
  Q_PROPERTY(int priority READ priority CONSTANT)
  Q_PROPERTY(quint64 arrivalTime READ arrivalTime CONSTANT)
  Q_PROPERTY(quint64 waitTime READ waitTime NOTIFY statsChanged)
  Q_PROPERTY(quint64 cpuTime READ cpuTime NOTIFY statsChanged)
  Q_PROPERTY(quint64 ioTime READ ioTime NOTIFY statsChanged)
  Q_PROPERTY(QString currentBurst READ currentBurst NOTIFY burstChanged)
  Q_PROPERTY(int pageFaults READ pageFaults NOTIFY statsChanged)
  Q_PROPERTY(int preemptions READ preemptions NOTIFY statsChanged)
  Q_PROPERTY(int requiredPages READ requiredPages CONSTANT)
  Q_PROPERTY(int nextPage READ nextPage NOTIFY statsChanged)
  Q_PROPERTY(QString referenceString READ referenceString CONSTANT)

 public:
  explicit ProcessItemModel(QObject* parent = nullptr);

  // Getters
  int pid() const { return m_pid; }
  QString state() const { return m_state; }
  int priority() const { return m_priority; }
  quint64 arrivalTime() const { return m_arrivalTime; }
  quint64 waitTime() const { return m_waitTime; }
  quint64 cpuTime() const { return m_cpuTime; }
  quint64 ioTime() const { return m_ioTime; }
  QString currentBurst() const { return m_currentBurst; }
  int pageFaults() const { return m_pageFaults; }
  int preemptions() const { return m_preemptions; }
  int requiredPages() const { return m_requiredPages; }
  int nextPage() const { return m_nextPage; }
  QString referenceString() const { return m_referenceString; }

  /**
   * @brief Actualiza los datos desde un Process del backend
   * @param process Puntero const al Process
   */
  void updateFrom(const waos::core::Process* process);

 signals:
  void stateChanged();
  void statsChanged();
  void burstChanged();

 private:
  int m_pid = 0;
  QString m_state;
  int m_priority = 0;
  quint64 m_arrivalTime = 0;
  quint64 m_waitTime = 0;
  quint64 m_cpuTime = 0;
  quint64 m_ioTime = 0;
  QString m_currentBurst;
  int m_pageFaults = 0;
  int m_preemptions = 0;
  int m_requiredPages = 0;
  int m_nextPage = 0;
  QString m_referenceString;

  QString stateToString(waos::core::ProcessState state) const;
};

}  // namespace waos::gui::models
