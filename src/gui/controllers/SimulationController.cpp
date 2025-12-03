#include "SimulationController.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>

#include "../viewmodels/BlockingEventsViewModel.h"
#include "../viewmodels/GanttViewModel.h"
#include "waos/memory/FIFOMemoryManager.h"
#include "waos/memory/LRUMemoryManager.h"
#include "waos/memory/OptimalMemoryManager.h"
#include "waos/scheduler/FCFSScheduler.h"
#include "waos/scheduler/PriorityScheduler.h"
#include "waos/scheduler/RRScheduler.h"
#include "waos/scheduler/SJFScheduler.h"

namespace waos::gui::controllers {

SimulationController::SimulationController(QObject* parent)
    : QObject(parent), m_simulator(std::make_unique<waos::core::Simulator>()), m_timer(new QTimer(this)) {
  // Initialize with defaults for integration testing
  m_simulator->setScheduler(std::make_unique<waos::scheduler::FCFSScheduler>());
  m_simulator->setMemoryManager(std::make_unique<waos::memory::FIFOMemoryManager>(16, m_simulator->getClockRef()));

  // Try to locate the test file
  QStringList candidatePaths = {
      "tests/mock/test_processes.txt",                                       // Run from project root
      "../../tests/mock/test_processes.txt",                                 // Run from build/
      "../tests/mock/test_processes.txt",                                    // Run from build/Debug/ etc
      "C:/Users/JhonAQ/Desktop/Projects/WaOS/tests/mock/test_processes.txt"  // Absolute fallback
  };

  bool loaded = false;
  for (const QString& path : candidatePaths) {
    if (QFileInfo::exists(path)) {
      qDebug() << "Loading processes from:" << path;
      if (m_simulator->loadProcesses(path.toStdString())) {
        loaded = true;
        break;
      }
    }
  }

  if (!loaded) {
    qWarning() << "Failed to load process file. Checked paths:" << candidatePaths;
  }

  connect(m_timer, &QTimer::timeout, this, &SimulationController::onTimeout);
}

void SimulationController::start() {
  if (!m_simulator->getAllProcesses().empty()) {
    m_simulator->start();
    m_timer->start(m_tickInterval);
    emit isRunningChanged();
  } else {
    qWarning() << "Cannot start simulation: No processes loaded.";
  }
}

void SimulationController::stop() {
  m_simulator->stop();
  m_timer->stop();
  emit isRunningChanged();
}

void SimulationController::reset() {
  stop();
  m_simulator->reset();

  // Reload processes
  QStringList candidatePaths = {
      "tests/mock/test_processes.txt",
      "../../tests/mock/test_processes.txt",
      "../tests/mock/test_processes.txt",
      "C:/Users/JhonAQ/Desktop/Projects/WaOS/tests/mock/test_processes.txt"};

  for (const QString& path : candidatePaths) {
    if (QFileInfo::exists(path)) {
      m_simulator->loadProcesses(path.toStdString());
      break;
    }
  }
  emit simulationReset();
}

void SimulationController::step() {
  // Force a single step even if paused
  m_simulator->tick(true);
}

void SimulationController::configure(const QString& scheduler, int quantum, const QString& memory, int frames, const QString& filePath) {
  // 1. Configure Scheduler
  if (scheduler == "Round Robin") {
    m_simulator->setScheduler(std::make_unique<waos::scheduler::RRScheduler>(quantum));
  } else if (scheduler == "SJF") {
    m_simulator->setScheduler(std::make_unique<waos::scheduler::SJFScheduler>());
  } else if (scheduler == "Priority") {
    m_simulator->setScheduler(std::make_unique<waos::scheduler::PriorityScheduler>());
  } else {
    // Default to FCFS
    m_simulator->setScheduler(std::make_unique<waos::scheduler::FCFSScheduler>());
  }

  // 2. Configure Memory Manager
  if (frames <= 0) frames = 16; // Default fallback

  if (memory == "LRU") {
    m_simulator->setMemoryManager(std::make_unique<waos::memory::LRUMemoryManager>(frames, m_simulator->getClockRef()));
  } else if (memory == "Optimal") {
    m_simulator->setMemoryManager(std::make_unique<waos::memory::OptimalMemoryManager>(frames, m_simulator->getClockRef()));
  } else {
    // Default to FIFO
    m_simulator->setMemoryManager(std::make_unique<waos::memory::FIFOMemoryManager>(frames, m_simulator->getClockRef()));
  }

  // 3. Load Process File
  if (!filePath.isEmpty()) {
    QString cleanPath = filePath;
    if (cleanPath.startsWith("file:///")) {
      cleanPath = cleanPath.remove(0, 8);
    }

    if (QFileInfo::exists(cleanPath)) {
      m_simulator->loadProcesses(cleanPath.toStdString());
    } else {
      qWarning() << "File not found:" << cleanPath;
    }
  }

  emit schedulerAlgorithmChanged();
  emit memoryAlgorithmChanged();
  emit simulationReset();  // Refresh views
}

bool SimulationController::isRunning() const {
  return m_timer->isActive();
}

int SimulationController::tickInterval() const {
  return m_tickInterval;
}

void SimulationController::setTickInterval(int interval) {
  if (m_tickInterval != interval) {
    m_tickInterval = interval;
    m_timer->setInterval(m_tickInterval);
    emit tickIntervalChanged();
  }
}

QString SimulationController::schedulerAlgorithm() const {
  return QString::fromStdString(m_simulator->getSchedulerAlgorithmName());
}

QString SimulationController::memoryAlgorithm() const {
  return QString::fromStdString(m_simulator->getMemoryAlgorithmName());
}

void SimulationController::registerProcessViewModel(waos::gui::viewmodels::ProcessMonitorViewModel* vm) {
  if (vm) {
    vm->setSimulator(m_simulator.get());
    connect(this, &SimulationController::simulationReset, vm, &waos::gui::viewmodels::ProcessMonitorViewModel::reset);
  }
}

void SimulationController::registerMemoryViewModel(waos::gui::viewmodels::MemoryMonitorViewModel* vm) {
  if (vm) {
    vm->setSimulator(m_simulator.get());
    connect(this, &SimulationController::simulationReset, vm, &waos::gui::viewmodels::MemoryMonitorViewModel::reset);
  }
}

void SimulationController::registerExecutionLogViewModel(waos::gui::viewmodels::ExecutionLogViewModel* vm) {
  if (vm) {
    vm->setSimulator(m_simulator.get());
    connect(this, &SimulationController::simulationReset, vm, &waos::gui::viewmodels::ExecutionLogViewModel::reset);
  }
}

void SimulationController::registerBlockingEventsViewModel(waos::gui::viewmodels::BlockingEventsViewModel* vm) {
  if (vm) {
    vm->setSimulator(m_simulator.get());
    connect(this, &SimulationController::simulationReset, vm, &waos::gui::viewmodels::BlockingEventsViewModel::reset);
  }
}

void SimulationController::registerGanttViewModel(waos::gui::viewmodels::GanttViewModel* vm) {
  if (vm) {
    vm->setSimulator(m_simulator.get());
    connect(this, &SimulationController::simulationReset, vm, &waos::gui::viewmodels::GanttViewModel::reset);
  }
}

void SimulationController::onTimeout() {
  m_simulator->tick();
}

}  // namespace waos::gui::controllers
