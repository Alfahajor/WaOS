#include "SimulationController.h"

#include "waos/memory/FIFOMemoryManager.h"
#include "waos/memory/LRUMemoryManager.h"
#include "waos/memory/OptimalMemoryManager.h"
#include "waos/scheduler/FCFSScheduler.h"
#include "waos/scheduler/PriorityScheduler.h"
#include "waos/scheduler/RRScheduler.h"
#include "waos/scheduler/SJFScheduler.h"

namespace waos::gui::controllers {

SimulationController::SimulationController(QObject* parent)
    : QObject(parent), m_simulator(std::make_unique<waos::core::Simulator>()) {
}

void SimulationController::loadProcesses(const QString& filePath) {
  if (!m_simulator->loadProcesses(filePath.toStdString())) {
    emit errorOccurred("Failed to load processes from " + filePath);
  }
}

void SimulationController::startSimulation() {
  m_simulator->start();
  emit isRunningChanged();
}

void SimulationController::pauseSimulation() {
  m_simulator->stop();
  emit isRunningChanged();
}

void SimulationController::resetSimulation() {
  m_simulator->reset();
  emit simulationReset();
  emit isRunningChanged();
}

void SimulationController::setScheduler(const QString& type, int quantum) {
  std::unique_ptr<waos::scheduler::IScheduler> scheduler;
  if (type == "FCFS") {
    scheduler = std::make_unique<waos::scheduler::FCFSScheduler>();
  } else if (type == "RR") {
    scheduler = std::make_unique<waos::scheduler::RRScheduler>(quantum);
  } else if (type == "SJF") {
    scheduler = std::make_unique<waos::scheduler::SJFScheduler>();
  } else if (type == "Priority") {
    scheduler = std::make_unique<waos::scheduler::PriorityScheduler>();
  }

  if (scheduler) {
    m_simulator->setScheduler(std::move(scheduler));
  }
}

void SimulationController::setMemoryManager(const QString& type, int totalFrames) {
  std::unique_ptr<waos::memory::IMemoryManager> memoryManager;
  if (type == "FIFO") {
    memoryManager = std::make_unique<waos::memory::FIFOMemoryManager>(totalFrames);
  } else if (type == "LRU") {
    memoryManager = std::make_unique<waos::memory::LRUMemoryManager>(totalFrames);
  } else if (type == "Optimal") {
    memoryManager = std::make_unique<waos::memory::OptimalMemoryManager>(totalFrames);
  }

  if (memoryManager) {
    m_simulator->setMemoryManager(std::move(memoryManager));
  }
}

bool SimulationController::isRunning() const {
  return m_simulator->isRunning();
}

waos::core::Simulator* SimulationController::getSimulator() const {
  return m_simulator.get();
}

}  // namespace waos::gui::controllers
