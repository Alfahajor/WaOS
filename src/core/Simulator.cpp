#include "waos/core/Simulator.h"

#include <iostream>
#include <waos/memory/IMemoryManager.h>
#include <waos/scheduler/IScheduler.h>
#include "waos/common/DataStructures.h"
#include "waos/core/Parser.h"

namespace waos::core {

Simulator::Simulator(QObject* parent)
    : QObject(parent),
      m_runningProcess(nullptr),
      m_nextProcess(nullptr),
      m_contextSwitchCounter(0),
      m_isKernelBusy(false),
      m_cpuActiveTicks(0),
      m_totalPageFaults(0),
      m_totalContextSwitches(0),
      m_isRunning(false) {
}

Simulator::~Simulator() { reset(); }

bool Simulator::loadProcesses(const std::string& filePath) {
  try {
    auto processInfos = Parser::parseFile(filePath);
    reset();

    for (const auto& info : processInfos) {
      auto process = std::make_unique<Process>(info.pid, info.arrivalTime, info.priority, info.bursts, info.requiredPages);
      m_incomingProcesses.push_back(process.get());
      m_processes.push_back(std::move(process));
    }
    // Ordenar por tiempo de llegada, luego PID
    std::sort(m_incomingProcesses.begin(), m_incomingProcesses.end(),
              [](Process* a, Process* b) {
                return (a->getArrivalTime() < b->getArrivalTime()) || 
                       (a->getArrivalTime() == b->getArrivalTime() && a->getPid() < b->getPid());
              });
    log(QString("Cargados %1 procesos.").arg(m_processes.size()), LogCategory::SYS);
    return true;
  } catch (const std::exception& e) {
    log(QString("Error carga: %1").arg(e.what()), LogCategory::SYS);
    return false;
  }
}

void Simulator::setScheduler(std::unique_ptr<waos::scheduler::IScheduler> scheduler) {
  m_scheduler = std::move(scheduler);
}

void Simulator::setMemoryManager(std::unique_ptr<waos::memory::IMemoryManager> manager) {
  m_memoryManager = std::move(manager);
}

void Simulator::start() {
  if (m_scheduler && m_memoryManager && !m_processes.empty()) {
    m_isRunning = true;
    log("Simulación iniciada.", LogCategory::SYS);
  }
}

void Simulator::stop() {
  m_isRunning = false;
  log("Simulación detenida.", LogCategory::SYS);
}

void Simulator::reset() {
  stop();
  for (auto& p : m_processes) p->stopThread();
  
  m_runningProcess = nullptr;
  m_nextProcess = nullptr;
  m_isKernelBusy = false;
  m_blockedQueue.clear();
  m_memoryWaitQueue.clear();
  m_processes.clear();
  m_incomingProcesses.clear();
  
  m_cpuActiveTicks = 0;
  m_totalPageFaults = 0;
  m_totalContextSwitches = 0;
  m_metrics = waos::common::SimulatorMetrics();
  
  if (m_memoryManager) m_memoryManager->reset();
  m_clock.reset();
  
  log("Simulación reiniciada.", LogCategory::SYS);
}

void Simulator::tick(bool force) {
  if (!m_isRunning && !force) return;
  std::lock_guard<std::recursive_mutex> lock(m_simulationMutex);
  step();
}

// Master Orchestation Logic
void Simulator::step() {
  uint64_t now = m_clock.getTime();
  bool potentialPreemption = false;

  // Estas funciones mueven procesos a READY, pero NO ejecutan CS aún.
  potentialPreemption |= handleArrivals();
  potentialPreemption |= handleIO();
  potentialPreemption |= handleMemoryOps();

  // Si algo nuevo llegó y tiene más prioridad, forzamos CS antes de ejecutar nada.
  if (!m_isKernelBusy && m_runningProcess != nullptr && potentialPreemption) {
    if (checkPreemption()) {
      // Al activar esto, m_isKernelBusy se vuelve true inmediatamente.
      // El tick actual se consumirá en el "else if (m_isKernelBusy)" de abajo.
    }
  }

  // Execution phase 
  if (m_isKernelBusy) {
    executeKernelCycle();
  } else {
    if (m_runningProcess == nullptr) {
      Process* next = m_scheduler->getNextProcess();
      if (next) {
        m_runningProcess = next;
        m_runningProcess->setState(ProcessState::RUNNING, now);
        emit processStateChanged(m_runningProcess->getPid(), ProcessState::RUNNING);
        log(QString("CPU asignada a P%1").arg(m_runningProcess->getPid()), LogCategory::SCHED);
      }
    }

    if (m_runningProcess != nullptr) {
      executeUserCycle();
    }
  }

  updateMetrics();
  emit clockTicked(now);
  m_clock.tick();
}

bool Simulator::handleArrivals() {
  bool newReady = false;
  uint64_t now = m_clock.getTime();
  
  auto it = m_incomingProcesses.begin();
  while (it != m_incomingProcesses.end()) {
    Process* p = *it;
    if (p->getArrivalTime() <= now) {
      p->startThread();
      m_memoryManager->allocateForProcess(p->getPid(), p->getRequiredPages());
      m_memoryManager->registerFutureReferences(p->getPid(), p->getPageReferenceString());

      p->setState(ProcessState::READY, now);
      emit processStateChanged(p->getPid(), ProcessState::READY);
      m_scheduler->addProcess(p);
      
      log(QString("Nuevo Proceso: P%1 (Prio %2)").arg(p->getPid()).arg(p->getPriority()), LogCategory::PROC);
      
      it = m_incomingProcesses.erase(it);
      newReady = true;
    } else {
      break; 
    }
  }
  return newReady;
}

bool Simulator::handleIO() {
  bool newReady = false;
  auto it = m_blockedQueue.begin();
  if (it != m_blockedQueue.end()) {
    Process* p = *it;
    bool done = p->simulateIoWait(1);
    p->addIoTime(1);
    
    if (done) {
      p->advanceToNextBurst();
      p->resetQuantum(); // New burst CPU, reset quantum

      p->setState(ProcessState::READY, m_clock.getTime());
      emit processStateChanged(p->getPid(), ProcessState::READY);
      m_scheduler->addProcess(p);
      
      log(QString("P%1 terminó E/S").arg(p->getPid()), LogCategory::NOTIFY);
      it = m_blockedQueue.erase(it);
      newReady = true;
    }
  }
  return newReady;
}

bool Simulator::handleMemoryOps() {
  bool newReady = false;
  auto it = m_memoryWaitQueue.begin();
  if (it != m_memoryWaitQueue.end()) {
    it->ticksRemaining--;
    it->process->addIoTime(1);
    
    if (it->ticksRemaining <= 0) {
      m_memoryManager->completePageLoad(it->process->getPid(), it->pageNumber);
      it->process->resetQuantum(); // Reset quantum to back
      
      it->process->setState(ProcessState::READY, m_clock.getTime());
      emit processStateChanged(it->process->getPid(), ProcessState::READY);
      m_scheduler->addProcess(it->process);
      
      log(QString("P%1 página %2 cargada").arg(it->process->getPid()).arg(it->pageNumber), LogCategory::MEM);
      it = m_memoryWaitQueue.erase(it);
      newReady = true;
    }
  }
  return newReady;
}

void Simulator::executeKernelCycle() {
  m_cpuActiveTicks++;
  m_contextSwitchCounter--;

  if (m_contextSwitchCounter <= 0) {
    completeContextSwitch();
  }
}

void Simulator::executeUserCycle() {
  // MMU Check (Antes de ejecutar)
  int pageReq = m_runningProcess->getCurrentPageRequirement();
  auto result = m_memoryManager->requestPage(m_runningProcess->getPid(), pageReq);

  if (result != waos::memory::PageRequestResult::HIT) {
    log(QString("Fallo de Página: P%1 Pag %2").arg(m_runningProcess->getPid()).arg(pageReq), LogCategory::MEM);
    m_runningProcess->incrementPageFaults();
    m_totalPageFaults++;
    
    m_memoryWaitQueue.push_back({m_runningProcess, m_pageFaultPenalty, pageReq});
    
    // Start Switch Out inmediately
    triggerContextSwitch(m_runningProcess, ProcessState::WAITING_MEMORY);
    return;
  }

  int durationBefore = m_runningProcess->getCurrentBurstDuration();

  // Normal execution
  m_systemMonitor.dispatch(m_runningProcess);
  m_systemMonitor.waitForBurstCompletion(m_runningProcess);

  int durationAfter = m_runningProcess->getCurrentBurstDuration();
  
  m_runningProcess->addCpuTime(1);
  m_runningProcess->incrementQuantum(1);
  m_memoryManager->advanceInstructionPointer(m_runningProcess->getPid());
  m_cpuActiveTicks++;

  std::cout << "[DEBUG] P" << m_runningProcess->getPid() 
            << " burst: " << durationBefore << " -> " << durationAfter << std::endl;

  // Post-Execution checks
  if (m_runningProcess->getCurrentBurstDuration() == 0) {
    m_runningProcess->advanceToNextBurst();

    if (!m_runningProcess->hasMoreBursts()) {
      log(QString("P%1 Terminado").arg(m_runningProcess->getPid()), LogCategory::PROC);
      m_memoryManager->freeForProcess(m_runningProcess->getPid());

      m_runningProcess->setState(ProcessState::TERMINATED, m_clock.getTime());
      emit processStateChanged(m_runningProcess->getPid(), ProcessState::TERMINATED);
      m_runningProcess = nullptr;
    } else if (m_runningProcess->getCurrentBurstType() == BurstType::IO) {
      log(QString("P%1 solicita E/S").arg(m_runningProcess->getPid()), LogCategory::SYS);
      m_blockedQueue.push_back(m_runningProcess);
      triggerContextSwitch(m_runningProcess, ProcessState::BLOCKED);
    } else {
      // Burst CPU seguido de otro Burst CPU (raro, pero yield)
      triggerContextSwitch(m_runningProcess, ProcessState::READY);
    }
  } else {
    // Check Quantum
    int timeSlice = m_scheduler->getTimeSlice();
    if (timeSlice > 0 && m_runningProcess->getQuantumUsed() >= timeSlice) {
      log(QString("Quantum expirado P%1").arg(m_runningProcess->getPid()), LogCategory::SCHED);
      m_runningProcess->incrementPreemptions();
      triggerContextSwitch(m_runningProcess, ProcessState::READY);
    }
  }
}

void Simulator::triggerContextSwitch(Process* current, ProcessState nextState, Process* next) {
  // Configurar estado de salida del proceso actual
  if (current) {
    current->setState(nextState, m_clock.getTime());
    emit processStateChanged(current->getPid(), nextState);
    if (nextState == ProcessState::READY) {
      current->resetQuantum();
      m_scheduler->addProcess(current);
    }
  }

  // Preparar Kernel para switch
  m_runningProcess = nullptr;
  m_nextProcess = next;       // Puede ser null, el kernel decidirá al terminar el switch
  m_isKernelBusy = true;
  m_contextSwitchCounter = m_contextSwitchDuration;
  m_totalContextSwitches++;
}

void Simulator::completeContextSwitch() {
  m_isKernelBusy = false;

  // Si no habíamos pre-seleccionado un proceso, preguntar al scheduler ahora
  if (!m_nextProcess) {
    m_nextProcess = m_scheduler->getNextProcess();
  }

  if (m_nextProcess) {
    m_runningProcess = m_nextProcess;
    m_nextProcess = nullptr;

    m_runningProcess->setState(ProcessState::RUNNING, m_clock.getTime());
    emit processStateChanged(m_runningProcess->getPid(), ProcessState::RUNNING);
    log(QString("CPU asignada a P%1").arg(m_runningProcess->getPid()), LogCategory::SCHED);
  } else {
    // Scheduler vacío, CPU queda IDLE
  }
}

bool Simulator::checkPreemption() {
  if (!m_runningProcess) return false;
  
  auto readyQueue = m_scheduler->peekReadyQueue();
  if (readyQueue.empty()) return false;

  const Process* candidate = readyQueue.front(); // Asume cola ordenada
  if (candidate->getPriority() < m_runningProcess->getPriority()) {
    log(QString("Preempción: P%1 desplaza a P%2").arg(candidate->getPid()).arg(m_runningProcess->getPid()), LogCategory::SCHED);
    m_runningProcess->incrementPreemptions();
    triggerContextSwitch(m_runningProcess, ProcessState::READY);
    return true;
  }
  return false;
}

void Simulator::updateMetrics() {
  m_metrics.currentTick = m_clock.getTime();
  m_metrics.totalProcesses = m_processes.size();
  m_metrics.totalPageFaults = m_totalPageFaults;
  m_metrics.totalContextSwitches = m_totalContextSwitches;

  if (m_metrics.currentTick > 0) {
    m_metrics.cpuUtilization = (double)m_cpuActiveTicks / m_metrics.currentTick * 100.0;
  }

  m_metrics.completedProcesses = 0;
  double totalWait = 0;
  double totalTurnaround = 0;

  for (const auto& p : m_processes) {
    if (p->getState() == ProcessState::TERMINATED) {
      m_metrics.completedProcesses++;
      auto stats = p->getStats();
      totalWait += stats.totalWaitTime;
      totalTurnaround += (stats.finishTime - p->getArrivalTime());
    }
  }

  if (m_metrics.completedProcesses > 0) {
    m_metrics.avgWaitTime = totalWait / m_metrics.completedProcesses;
    m_metrics.avgTurnaroundTime = totalTurnaround / m_metrics.completedProcesses;
  }

  if (m_isRunning && m_metrics.completedProcesses == m_processes.size() && !m_processes.empty()) {
    stop();
    emit simulationFinished();
    log("Todos los procesos terminados.", LogCategory::SYS);
  }
}

// Implementación de Getters
std::vector<const Process*> Simulator::getAllProcesses() const {
  std::vector<const Process*> v;
  for(auto& p : m_processes) v.push_back(p.get());
  return v;
}

const Process* Simulator::getRunningProcess() const { return m_runningProcess; }

std::vector<const Process*> Simulator::getBlockedProcesses() const {
  std::vector<const Process*> v;
  for(auto* p : m_blockedQueue) v.push_back(p);
  return v;
}

std::vector<waos::common::MemoryWaitInfo> Simulator::getMemoryWaitQueue() const {
  std::vector<waos::common::MemoryWaitInfo> v;
  for(auto& i : m_memoryWaitQueue) v.push_back({i.process->getPid(), i.pageNumber, i.ticksRemaining});
  return v;
}

std::vector<const Process*> Simulator::getReadyProcesses() const {
  if(m_scheduler) return m_scheduler->peekReadyQueue();
  return {};
}

waos::common::SimulatorMetrics Simulator::getSimulatorMetrics() const { return m_metrics; }
const waos::scheduler::IScheduler* Simulator::getScheduler() const { return m_scheduler.get(); }
const waos::memory::IMemoryManager* Simulator::getMemoryManager() const { return m_memoryManager.get(); }

std::string Simulator::getSchedulerAlgorithmName() const {
  if (m_scheduler) return m_scheduler->getAlgorithmName();
  return "None";
}

std::string Simulator::getMemoryAlgorithmName() const {
  if (m_memoryManager) return m_memoryManager->getAlgorithmName();
  return "None";
}

// Memory wrappers
std::vector<waos::common::FrameInfo> Simulator::getFrameStatus() const {
  if(m_memoryManager) return m_memoryManager->getFrameStatus(); return {}; 
}
std::vector<waos::common::PageTableEntryInfo> Simulator::getPageTableForProcess(int pid) const {
  if(m_memoryManager) return m_memoryManager->getPageTableForProcess(pid); return {};
}
waos::common::MemoryStats Simulator::getMemoryStats() const {
  if(m_memoryManager) return m_memoryManager->getMemoryStats(); return {};
}

// Log helper
void Simulator::log(const QString& message, LogCategory category) {
  uint64_t time = m_clock.getTime();
  QString timeStr = QString("%1:%2").arg(time / 60, 2, 10, QChar('0')).arg(time % 60, 2, 10, QChar('0'));
  
  QString color = "#a6e3a1"; // SYS default
  QString catStr = "SYS";
  switch (category) {
    case LogCategory::MEM: catStr = "MEM"; color = "#89b4fa"; break;
    case LogCategory::WAIT: catStr = "WAIT"; color = "#fab387"; break;
    case LogCategory::NOTIFY: catStr = "NOTIFY"; color = "#f9e2af"; break;
    case LogCategory::SCHED: catStr = "SCHED"; color = "#cba6f7"; break;
    case LogCategory::PROC: catStr = "PROC"; color = "#f5c2e7"; break;
    default: break;
  }
  
  emit logMessage(QString("<font color='#7f849c'>%1</font> <b><font color='%2'>%3:</font></b> %4")
                  .arg(timeStr, color, catStr, message));
}

// Getters técnicos para tests
uint64_t Simulator::getCurrentTime() const { return m_clock.getTime(); }
const uint64_t* Simulator::getClockRef() const { return m_clock.getTimeAddress(); }
bool Simulator::isRunning() const { return m_isRunning; }
bool Simulator::isContextSwitching() const { return m_isKernelBusy; }

}
