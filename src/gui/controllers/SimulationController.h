#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include "waos/core/Simulator.h"

namespace waos::gui::controllers {

class SimulationController : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)

 public:
  explicit SimulationController(QObject* parent = nullptr);

  Q_INVOKABLE void loadProcesses(const QString& filePath);
  Q_INVOKABLE void startSimulation();
  Q_INVOKABLE void pauseSimulation();
  Q_INVOKABLE void resetSimulation();
  Q_INVOKABLE void setScheduler(const QString& type, int quantum = 0);
  Q_INVOKABLE void setMemoryManager(const QString& type, int totalFrames = 0);

  bool isRunning() const;
  waos::core::Simulator* getSimulator() const;

 signals:
  void isRunningChanged();
  void errorOccurred(const QString& message);
  void simulationReset();

 private:
  std::unique_ptr<waos::core::Simulator> m_simulator;
};

}  // namespace waos::gui::controllers
