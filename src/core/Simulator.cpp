#include "waos/core/Simulator.h"

#include <waos/memory/IMemoryManager.h>
#include <waos/scheduler/IScheduler.h>

#include <algorithm>
#include <iostream>

#include "waos/common/DataStructures.h"
#include "waos/core/Parser.h"

namespace waos::core {

Simulator::Simulator(QObject* parent)
    : QObject(parent),
      m_runningProcess(nullptr),
      m_nextProcess(nullptr),
      m_contextSwitchCounter(0),
      m_cpuActiveTicks(0),
      m_totalPageFaults(0),
      m_totalContextSwitches(0),
      m_isRunning(false),
      m_pageFaultPenalty(5),
      m_contextSwitchDuration(1) {
}

Simulator::~Simulator() {
  // Ensure all threads are stopped on destruction
  reset();
}

bool Simulator::loadProcesses(const std::string& filePath) {
  try {
    auto processInfos = Parser::parseFile(filePath);

    // Clear existing data
    m_processes.clear();
    m_incomingProcesses.clear();
    m_blockedQueue.clear();
    m_memoryWaitQueue.clear();
    m_runningProcess = nullptr;
    m_nextProcess = nullptr;
    m_contextSwitchCounter = 0;

    // Reset Metrics accumulators
    m_cpuActiveTicks = 0;
    m_totalPageFaults = 0;
    m_totalContextSwitches = 0;

    // Convert ProcessInfo (DTO) to Process (Entity)
    for (const auto& info : processInfos) {
      auto process = std::make_unique<Process>(
          info.pid,
          info.arrivalTime,
          info.priority,
          info.bursts,
          info.requiredPages);

      // Store raw pointer in incoming list for arrival checks
      m_incomingProcesses.push_back(process.get());

      // Move ownership to the main vector
      m_processes.push_back(std::move(process));
    }

    // Sort incoming processes by arrival time for efficiency
    std::sort(m_incomingProcesses.begin(), m_incomingProcesses.end(),
              [](Process* a, Process* b) {
                if (a->getArrivalTime() != b->getArrivalTime()) {
                  return a->getArrivalTime() < b->getArrivalTime();
                }
                return a->getPid() < b->getPid();
              });

    log(QString("Se cargaron %1 procesos desde el archivo.").arg(m_processes.size()), LogCategory::SYS);
    return true;

  } catch (const std::exception& e) {
    log(QString("Error al cargar procesos: %1").arg(e.what()), LogCategory::SYS);
    return false;
  }
}

void Simulator::setScheduler(std::unique_ptr<waos::scheduler::IScheduler> scheduler) {
  m_scheduler = std::move(scheduler);
}

void Simulator::setMemoryManager(std::unique_ptr<waos::memory::IMemoryManager> memoryManager) {
  m_memoryManager = std::move(memoryManager);
}

void Simulator::start() {
  if (!m_scheduler || !m_memoryManager) {
    log("Error: Planificador o Gestor de Memoria no inicializado.", LogCategory::SYS);
    return;
  }

  if (m_processes.empty()) {
    log("Error: No hay procesos cargados.", LogCategory::SYS);
    return;
  }

  m_isRunning = true;
  log("Simulación iniciada.", LogCategory::SYS);
}

void Simulator::stop() {
  m_isRunning = false;
  log("Simulación detenida.", LogCategory::SYS);
}

void Simulator::reset() {
  stop();

  for (auto& process : m_processes) {
    if (process) {
      process->stopThread();  // Joins the thread
    }
  }

  m_runningProcess = nullptr;
  m_nextProcess = nullptr;
  m_contextSwitchCounter = 0;
  m_blockedQueue.clear();
  m_memoryWaitQueue.clear();

  // Reset Metrics
  m_cpuActiveTicks = 0;
  m_totalPageFaults = 0;
  m_totalContextSwitches = 0;
  m_metrics = waos::common::SimulatorMetrics();

  // Reset Memory Manager
  if (m_memoryManager) {
    m_memoryManager->reset();
  }

  // Clear main container (Destructors will run, but threads are already joined)
  m_processes.clear();
  m_incomingProcesses.clear();

  log("Simulación reiniciada.", LogCategory::SYS);
}

void Simulator::tick(bool force) {
  if (!m_isRunning && !force) return;
  step();
}

void Simulator::step() {
  // std::lock_guard<std::recursive_mutex> lock(m_simulationMutex);
  std::cout << "[DEBUG] Simulator::step start" << std::endl;

  uint64_t now = m_clock.getTime();
  emit clockTicked(now);

  // IO Devices (Parallel to CPU)
  handleIO();

  // Memory Disk Operations (Parallel to CPU)
  handlePageFaults();

  // Current running process executes its burst for this tick
  if (m_contextSwitchCounter > 0) {
    // Context Switch Overhead, CPU is busy doing kernel work
    m_contextSwitchCounter--;
    if (m_contextSwitchCounter == 0 && m_nextProcess) {
      m_runningProcess = m_nextProcess;
      m_nextProcess = nullptr;

      m_runningProcess->setState(ProcessState::RUNNING, m_clock.getTime());

      emit processStateChanged(m_runningProcess->getPid(), ProcessState::RUNNING);
      log(QString("Cambio de contexto completado. Ejecutando P%1").arg(m_runningProcess->getPid()), LogCategory::SCHED);
    }
  } else {
    // CPU is free for user process
    handleCpuExecution();
  }

  // Process Arrivals (May cause Preemption)
  handleArrivals();

  // Scheduling logic runs if CPU is free AND no switch is in progress.
  if (m_runningProcess == nullptr && m_contextSwitchCounter == 0) handleScheduling();

  updateMetrics();
  m_clock.tick();
  // std::cout << "[DEBUG] Simulator::step end" << std::endl;
}

void Simulator::handleArrivals() {
  uint64_t now = m_clock.getTime();
  auto it = m_incomingProcesses.begin();

  while (it != m_incomingProcesses.end()) {
    Process* p = *it;
    if (p->getArrivalTime() <= now) {
      // Start the OS thread for this process
      p->startThread();

      // Reserve structures
      m_memoryManager->allocateForProcess(p->getPid(), p->getRequiredPages());
      m_memoryManager->registerFutureReferences(p->getPid(), p->getPageReferenceString());

      // Move to READY (Scheduler se encarga de la cola)
      p->setState(ProcessState::READY, now);
      emit processStateChanged(p->getPid(), ProcessState::READY);
      m_scheduler->addProcess(p);

      // Preemption check
      Process* current = (m_runningProcess) ? m_runningProcess : m_nextProcess;
      if (current && p->getPriority() < current->getPriority()) {
        // New process has higher priority (lower value)
        log(QString("Apropiación: P%1 (Prio %2) desplaza a P%3 (Prio %4)")
                .arg(p->getPid())
                .arg(p->getPriority())
                .arg(current->getPid())
                .arg(current->getPriority()),
            LogCategory::SCHED);

        triggerContextSwitch(current, nullptr);  // Put current back to ready
        // The scheduler will pick the new high-priority process in handleScheduling
      }

      it = m_incomingProcesses.erase(it);
      log(QString("Proceso P%1 llegó.").arg(p->getPid()), LogCategory::PROC);
    } else {
      // Como están ordenados, si este no llegó, los siguientes tampoco.
      break;
    }
  }
}

bool Simulator::processIoStep(Process* p) {
  if (!p) return false;

  // Thread is blocked. We manually decrement burst duration.
  bool burstFinished = p->simulateIoWait(1);
  p->addIoTime(1);

  return burstFinished;
}

void Simulator::handleIO() {
  // IO handling remains simulated in kernel space for simplicity
  // and determinism, even with threaded processes. The thread is sleeping.
  auto it = m_blockedQueue.begin();
  if (it != m_blockedQueue.end()) {
    Process* p = *it;

    // Thread is conceptually blocked. Kernel updates the PCB state.
    // We manually decrement burst duration here because the thread is paused.
    bool burstFinished = processIoStep(p);

    if (burstFinished) {
      p->advanceToNextBurst();

      // Reset Quantum on I/O Completion (New scheduler eligibility)
      p->resetQuantum();

      // Back to READY
      p->setState(ProcessState::READY, m_clock.getTime());
      emit processStateChanged(p->getPid(), ProcessState::READY);
      m_scheduler->addProcess(p);

      // Preemption on IO Completion could also happen here for Priority Scheduling
      // We omit it for simplicity, but it follows the same logic as Arrivals.

      it = m_blockedQueue.erase(it);
      log(QString("Proceso P%1 terminó E/S.").arg(p->getPid()), LogCategory::NOTIFY);
    }
  }
}

void Simulator::handlePageFaults() {
  // Kernel simulates disk latency.
  auto it = m_memoryWaitQueue.begin();
  if (it != m_memoryWaitQueue.end()) {
    InternalMemoryWait& info = *it;

    // Simular tiempo de disco para cargar la página
    info.ticksRemaining--;
    if (info.process) info.process->addIoTime(1);  // Count disk wait as IO Time

    if (info.ticksRemaining <= 0) {
      // Notify to MemoryManager que la carga física is finished.
      m_memoryManager->completePageLoad(info.process->getPid(), info.pageNumber);

      // Reset Quantum on Fault Resolution
      info.process->resetQuantum();

      info.process->setState(ProcessState::READY, m_clock.getTime());
      emit processStateChanged(info.process->getPid(), ProcessState::READY);
      m_scheduler->addProcess(info.process);

      log(QString("Proceso P%1 resolvió Fallo de Página.").arg(info.process->getPid()), LogCategory::MEM);
      it = m_memoryWaitQueue.erase(it);
    }
  }
}

void Simulator::handleCpuExecution() {
  if (!m_runningProcess) return;

  // MMU Check (Hardware Instruction Fetch simulation)
  int pageRequired = m_runningProcess->getCurrentPageRequirement();

  // Request page - this counts hits AND faults
  waos::memory::PageRequestResult result = m_memoryManager->requestPage(m_runningProcess->getPid(), pageRequired);

  if (result != waos::memory::PageRequestResult::HIT) {
    // Page Fault Exception (either PAGE_FAULT or REPLACEMENT)
    log(QString("Fallo de Página durante ejecución: P%1 necesita Página %2")
            .arg(m_runningProcess->getPid())
            .arg(pageRequired),
        LogCategory::MEM);

    m_runningProcess->incrementPageFaults();
    m_totalPageFaults++;

    m_runningProcess->setState(ProcessState::WAITING_MEMORY, m_clock.getTime());
    emit processStateChanged(m_runningProcess->getPid(), ProcessState::WAITING_MEMORY);

    m_memoryWaitQueue.push_back({m_runningProcess, m_pageFaultPenalty, pageRequired});
    m_runningProcess = nullptr;  // Immediate yield on fault
    return;                      // Tick used for the faulting instruction attempt
  }
  // else: Page HIT - continue execution

  // ORCHESTRATION: Wake up the thread
  m_systemMonitor.dispatch(m_runningProcess);

  // BARRIER: Wait for the thread to finish its tick logic
  m_systemMonitor.waitForBurstCompletion(m_runningProcess);

  // If we reached here, the process successfully executed one tick of CPU burst.
  m_cpuActiveTicks++;

  // Post-Execution Kernel Accounting
  m_runningProcess->addCpuTime(1);
  m_runningProcess->incrementQuantum(1);

  // Advance instruction pointer in memory manager (for optimal algorithm)
  m_memoryManager->advanceInstructionPointer(m_runningProcess->getPid());

  // Check Burst Completion (Thread updated the queue)
  // We check the result of the thread's work.
  int remaining = m_runningProcess->getCurrentBurstDuration();

  if (remaining == 0) {
    m_runningProcess->advanceToNextBurst();

    if (!m_runningProcess->hasMoreBursts()) {
      m_runningProcess->setState(ProcessState::TERMINATED, m_clock.getTime());
      emit processStateChanged(m_runningProcess->getPid(), ProcessState::TERMINATED);
      log(QString("Proceso P%1 Terminado.").arg(m_runningProcess->getPid()), LogCategory::PROC);

      // Thread cleanup
      m_runningProcess->stopThread();
      m_memoryManager->freeForProcess(m_runningProcess->getPid());

      m_runningProcess = nullptr;
    } else {
      if (m_runningProcess->getCurrentBurstType() == BurstType::IO) {
        m_runningProcess->setState(ProcessState::BLOCKED, m_clock.getTime());
        emit processStateChanged(m_runningProcess->getPid(), ProcessState::BLOCKED);
        m_blockedQueue.push_back(m_runningProcess);
        m_runningProcess = nullptr;
      } else {
        // Sigue siendo CPU (caso raro de CPU consecutiva o retorno de interrupción)
        // For now, treat as yield to re-evaluate priorities/quantum
        triggerContextSwitch(m_runningProcess, nullptr);
      }
    }
  } else {
    // Burst not finished, check Quantum (Preemption)
    int timeSlice = m_scheduler->getTimeSlice();

    // Only apply quantum if scheduler uses time-slicing (timeSlice > 0)
    if (timeSlice > 0 && m_runningProcess->getQuantumUsed() >= timeSlice) {
      log(QString("Quantum expirado para P%1").arg(m_runningProcess->getPid()), LogCategory::SCHED);
      m_runningProcess->incrementPreemptions();
      triggerContextSwitch(m_runningProcess, nullptr);
    }
  }
}

void Simulator::handleScheduling() {
  if (!m_scheduler->hasReadyProcesses()) return;
  Process* candidate = m_scheduler->getNextProcess();

  if (!candidate) {
    log("Advertencia: El planificador devolvió nulo a pesar de reportar procesos listos.", LogCategory::SYS);
    return;
  }

  // Initiate Context Switch
  m_runningProcess = candidate;
  m_runningProcess->setState(ProcessState::RUNNING, m_clock.getTime());

  emit processStateChanged(m_runningProcess->getPid(), ProcessState::RUNNING);
  log(QString("Planificador seleccionó P%1. Iniciando inmediatamente (Sin sobrecarga de CC)").arg(candidate->getPid()), LogCategory::SCHED);
}

void Simulator::triggerContextSwitch(Process* current, Process* next) {
  bool isPreemption = (current != nullptr && current->getState() != ProcessState::TERMINATED);

  if (current) {
    // Reset Quantum on Preemption/Yield (Returning to Ready)
    current->resetQuantum();

    current->setState(ProcessState::READY, m_clock.getTime());
    emit processStateChanged(current->getPid(), ProcessState::READY);
    m_scheduler->addProcess(current);
  }
  m_runningProcess = nullptr;

  // If we have a specific next process (direct switch), set it up
  // Otherwise, set running to null so handleScheduling picks one
  if (isPreemption) {
    m_nextProcess = next;
    m_contextSwitchCounter = m_contextSwitchDuration;
    m_totalContextSwitches++;
  } else {
    // Immediate switch for non-preemptive cases
    if (next) {
      m_runningProcess = next;
      m_runningProcess->setState(ProcessState::RUNNING, m_clock.getTime());
      emit processStateChanged(m_runningProcess->getPid(), ProcessState::RUNNING);
    }
    // If next is null, handleScheduling will pick one immediately in step()
    m_contextSwitchCounter = 0;
  }
}

uint64_t Simulator::getCurrentTime() const { return m_clock.getTime(); }

const uint64_t* Simulator::getClockRef() const { return m_clock.getTimeAddress(); }

bool Simulator::isRunning() const { return m_isRunning; }

void Simulator::updateMetrics() {
  m_metrics.currentTick = m_clock.getTime();
  m_metrics.totalProcesses = m_processes.size();

  m_metrics.totalPageFaults = m_totalPageFaults;
  m_metrics.totalContextSwitches = m_totalContextSwitches;

  // Calculate CPU Utilization
  if (m_metrics.currentTick > 0) {
    m_metrics.cpuUtilization = (double)m_cpuActiveTicks / m_metrics.currentTick * 100.0;
  } else {
    m_metrics.cpuUtilization = 0.0;
  }

  // Process-specific stats still require iteration, but simpler logic
  m_metrics.completedProcesses = 0;
  double totalWait = 0;
  double totalTurnaround = 0;

  for (const auto& p : m_processes) {
    if (p->getState() == ProcessState::TERMINATED) {
      auto stats = p->getStats();
      m_metrics.completedProcesses++;
      totalWait += stats.totalWaitTime;
      totalTurnaround += (stats.finishTime - p->getArrivalTime());
    }
  }

  if (m_metrics.completedProcesses > 0) {
    m_metrics.avgWaitTime = totalWait / m_metrics.completedProcesses;
    m_metrics.avgTurnaroundTime = totalTurnaround / m_metrics.completedProcesses;
  } else {
    m_metrics.avgWaitTime = 0.0;
    m_metrics.avgTurnaroundTime = 0.0;
  }
}

// APIs to GUI
std::vector<const Process*> Simulator::getAllProcesses() const {
  // std::lock_guard<std::recursive_mutex> lock(m_simulationMutex);
  std::vector<const Process*> result;
  for (const auto& p : m_processes) result.push_back(p.get());
  return result;
}

const Process* Simulator::getRunningProcess() const {
  // std::lock_guard<std::recursive_mutex> lock(m_simulationMutex);
  return m_runningProcess;
}

std::vector<const Process*> Simulator::getBlockedProcesses() const {
  // std::lock_guard<std::recursive_mutex> lock(m_simulationMutex);
  std::vector<const Process*> result;
  for (const auto* p : m_blockedQueue) result.push_back(p);
  return result;
}

std::vector<waos::common::MemoryWaitInfo> Simulator::getMemoryWaitQueue() const {
  // std::lock_guard<std::recursive_mutex> lock(m_simulationMutex);
  std::vector<waos::common::MemoryWaitInfo> result;
  for (const auto& item : m_memoryWaitQueue) {
    result.push_back({item.process->getPid(), item.pageNumber, item.ticksRemaining});
  }
  return result;
}

std::vector<const Process*> Simulator::getReadyProcesses() const {
  // std::lock_guard<std::recursive_mutex> lock(m_simulationMutex);
  if (m_scheduler) return m_scheduler->peekReadyQueue();
  return {};
}

waos::common::SimulatorMetrics Simulator::getSimulatorMetrics() const {
  // std::lock_guard<std::recursive_mutex> lock(m_simulationMutex);
  return m_metrics;
}

std::string Simulator::getSchedulerAlgorithmName() const {
  if (m_scheduler) return m_scheduler->getAlgorithmName();
  return "None";
}

std::string Simulator::getMemoryAlgorithmName() const {
  if (m_memoryManager) return m_memoryManager->getAlgorithmName();
  return "None";
}

const waos::scheduler::IScheduler* Simulator::getScheduler() const {
  return m_scheduler.get();
}

const waos::memory::IMemoryManager* Simulator::getMemoryManager() const {
  return m_memoryManager.get();
}

std::vector<waos::common::FrameInfo> Simulator::getFrameStatus() const {
  // std::lock_guard<std::recursive_mutex> lock(m_simulationMutex);
  if (m_memoryManager) return m_memoryManager->getFrameStatus();
  return {};
}

std::vector<waos::common::PageTableEntryInfo> Simulator::getPageTableForProcess(int processId) const {
  // std::lock_guard<std::recursive_mutex> lock(m_simulationMutex);
  if (m_memoryManager) return m_memoryManager->getPageTableForProcess(processId);
  return {};
}

waos::common::MemoryStats Simulator::getMemoryStats() const {
  // std::lock_guard<std::recursive_mutex> lock(m_simulationMutex);
  if (m_memoryManager) return m_memoryManager->getMemoryStats();
  return {};
}

void Simulator::log(const QString& message, LogCategory category) {
  uint64_t time = m_clock.getTime();
  QString timeStr = QString("%1:%2")
                        .arg(time / 60, 2, 10, QChar('0'))
                        .arg(time % 60, 2, 10, QChar('0'));

  QString catStr;
  QString color;

  switch (category) {
    case LogCategory::SYS:
      catStr = "SYS";
      color = "#a6e3a1";  // Green
      break;
    case LogCategory::MEM:
      catStr = "MEM";
      color = "#89b4fa";  // Blue
      break;
    case LogCategory::WAIT:
      catStr = "WAIT";
      color = "#fab387";  // Orange
      break;
    case LogCategory::NOTIFY:
      catStr = "NOTIFY";
      color = "#f9e2af";  // Yellow
      break;
    case LogCategory::SCHED:
      catStr = "SCHED";
      color = "#cba6f7";  // Purple
      break;
    case LogCategory::PROC:
      catStr = "PROC";
      color = "#f5c2e7";  // Pink
      break;
  }

  QString formattedMessage = QString("<font color='#7f849c'>%1</font> &nbsp; <b><font color='%2'>%3:</font></b> %4")
                                 .arg(timeStr)
                                 .arg(color)
                                 .arg(catStr)
                                 .arg(message);

  emit logMessage(formattedMessage);
}

}  // namespace waos::core
