#pragma once
#include <QList>
#include <QObject>

#include "../models/ProcessItemModel.h"
#include "waos/core/Process.h"
#include "waos/core/Simulator.h"

namespace waos::gui::viewmodels {

class ProcessMonitorViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(QList<QObject*> processList READ processList NOTIFY processListChanged)
  Q_PROPERTY(double avgWaitTime READ avgWaitTime NOTIFY avgWaitTimeChanged)
  Q_PROPERTY(double avgTurnaroundTime READ avgTurnaroundTime NOTIFY avgTurnaroundTimeChanged)
  Q_PROPERTY(double cpuUtilization READ cpuUtilization NOTIFY cpuUtilizationChanged)

 public:
  explicit ProcessMonitorViewModel(QObject* parent = nullptr);

  void setSimulator(waos::core::Simulator* simulator);
  QList<QObject*> processList() const { return m_processItems; }

  double avgWaitTime() const { return m_avgWaitTime; }
  double avgTurnaroundTime() const { return m_avgTurnaroundTime; }
  double cpuUtilization() const { return m_cpuUtilization; }

 public slots:
  void onClockTicked(uint64_t tick);

 signals:
  void processListChanged();
  void avgWaitTimeChanged();
  void avgTurnaroundTimeChanged();
  void cpuUtilizationChanged();

 private:
  waos::core::Simulator* m_simulator = nullptr;
  QList<QObject*> m_processItems;

  double m_avgWaitTime = 0.0;
  double m_avgTurnaroundTime = 0.0;
  double m_cpuUtilization = 0.0;
};

}  // namespace waos::gui::viewmodels
