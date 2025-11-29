#pragma once

#include <QObject>
#include <QTimer>
#include <memory>

#include "../mock/MockSimulator.h"
#include "../viewmodels/MemoryMonitorViewModel.h"
#include "../viewmodels/ProcessMonitorViewModel.h"

namespace waos::gui::controllers {

class SimulationController : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
  Q_PROPERTY(int tickInterval READ tickInterval WRITE setTickInterval NOTIFY tickIntervalChanged)

 public:
  explicit SimulationController(QObject* parent = nullptr);

  Q_INVOKABLE void start();
  Q_INVOKABLE void stop();
  Q_INVOKABLE void reset();
  Q_INVOKABLE void step();

  bool isRunning() const;
  int tickInterval() const;
  void setTickInterval(int interval);

  void registerProcessViewModel(waos::gui::viewmodels::ProcessMonitorViewModel* vm);
  void registerMemoryViewModel(waos::gui::viewmodels::MemoryMonitorViewModel* vm);

 signals:
  void isRunningChanged();
  void tickIntervalChanged();

 private slots:
  void onTimeout();

 private:
  std::unique_ptr<waos::gui::mock::MockSimulator> m_simulator;
  QTimer* m_timer;
  int m_tickInterval = 1000;
};

}  // namespace waos::gui::controllers
