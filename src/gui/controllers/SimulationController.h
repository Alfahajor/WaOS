#pragma once

#include <QObject>
#include <QTimer>
#include <memory>

#include "../viewmodels/BlockingEventsViewModel.h"
#include "../viewmodels/ExecutionLogViewModel.h"
#include "../viewmodels/MemoryMonitorViewModel.h"
#include "../viewmodels/ProcessMonitorViewModel.h"
#include "waos/core/Simulator.h"

namespace waos::gui::controllers {

class SimulationController : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
  Q_PROPERTY(int tickInterval READ tickInterval WRITE setTickInterval NOTIFY tickIntervalChanged)
  Q_PROPERTY(QString schedulerAlgorithm READ schedulerAlgorithm NOTIFY schedulerAlgorithmChanged)
  Q_PROPERTY(QString memoryAlgorithm READ memoryAlgorithm NOTIFY memoryAlgorithmChanged)

 public:
  explicit SimulationController(QObject* parent = nullptr);

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  Q_INVOKABLE void reset();
  Q_INVOKABLE void step();

  bool isRunning() const;
  int tickInterval() const;
  void setTickInterval(int interval);

  QString schedulerAlgorithm() const;
  QString memoryAlgorithm() const;

  void registerProcessViewModel(waos::gui::viewmodels::ProcessMonitorViewModel* vm);
  void registerMemoryViewModel(waos::gui::viewmodels::MemoryMonitorViewModel* vm);
  void registerExecutionLogViewModel(waos::gui::viewmodels::ExecutionLogViewModel* vm);
  void registerBlockingEventsViewModel(waos::gui::viewmodels::BlockingEventsViewModel* vm);

 signals:
  void isRunningChanged();
  void tickIntervalChanged();
  void schedulerAlgorithmChanged();
  void memoryAlgorithmChanged();
  void simulationReset();

 private slots:
  void onTimeout();

 private:
  std::unique_ptr<waos::core::Simulator> m_simulator;
  QTimer* m_timer;
  int m_tickInterval = 1000;
};

}  // namespace waos::gui::controllers
