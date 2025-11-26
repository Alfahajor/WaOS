#include "waos/core/Simulator.h"
#include "waos/core/Parser.h"
#include <waos/scheduler/IScheduler.h>
#include <waos/memory/IMemoryManager.h>
#include <iostream>
#include <algorithm>

namespace waos::core {

  Simulator::Simulator(QObject* parent)
    : QObject(parent),
      m_runningProcess(nullptr),
      m_nextProcess(nullptr),
      m_contextSwitchCounter(0),
      m_isRunning(false) {
  }

  Simulator::~Simulator() {
    // Ensure all threads are stopped on destruction
    stop();
    m_processes.clear(); // Trigger Process destructors to join threads
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

      // Convert ProcessInfo (DTO) to Process (Entity)
      for (const auto& info : processInfos) {
        auto process = std::make_unique<Process>(
          info.pid,
          info.arrivalTime,
          info.priority,
          info.bursts,
          info.requiredPages
        );

        // Store raw pointer in incoming list for arrival checks
        m_incomingProcesses.push_back(process.get());

        // Move ownership to the main vector
        m_processes.push_back(std::move(process));
      }

      // Sort incoming processes by arrival time for efficiency
      std::sort(m_incomingProcesses.begin(), m_incomingProcesses.end(),
                [](Process* a, Process* b) {
                  return a->getArrivalTime() < b->getArrivalTime();
                });

      emit logMessage(QString("Loaded %1 processes from file.").arg(m_processes.size()));
      return true;

    } catch (const std::exception& e) {
      emit logMessage(QString("Error loading processes: %1").arg(e.what()));
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
      emit logMessage("Error: Scheduler or Memory Manager not initialized.");
      return;
    }

    if (m_processes.empty()) {
      emit logMessage("Error: No processes loaded.");
      return;
    }

    m_isRunning = true;
    emit logMessage("Simulation started.");
  }

  void Simulator::stop() {
    m_isRunning = false;
    emit logMessage("Simulation stopped.");
  }

  void Simulator::reset() {
    stop();
    m_runningProcess = nullptr;
    m_nextProcess = nullptr;
    m_contextSwitchCounter = 0;
    m_blockedQueue.clear();
    m_memoryWaitQueue.clear(); 

    // Re-create processes or reset logic would be needed here for full reset
    emit logMessage("Simulation reset.");
  }

  void Simulator::tick() {
    if (!m_isRunning) return;
    step();
  }

  uint64_t Simulator::getCurrentTime() const { return m_clock.getTime(); }

  bool Simulator::isRunning() const { return m_isRunning; }

  void Simulator::step() {
    std::lock_guard<std::mutex> lock(m_simulationMutex);

    uint64_t now = m_clock.getTime();
    emit clockTicked(now);

    // Process Arrivals (May cause Preemption)
    handleArrivals();

    // IO Devices (Parallel to CPU)
    handleIO();

    // Memory Disk Operations (Parallel to CPU)
    handlePageFaults();

    // Kernel / CPU Execution (Orchestration)
    if (m_contextSwitchCounter > 0) {
      // Context Switch Overhead, CPU is busy doing kernel work
      m_contextSwitchCounter--;
      if (m_contextSwitchCounter == 0 && m_nextProcess) {
        m_runningProcess = m_nextProcess;
        m_nextProcess = nullptr;

        m_runningProcess->setState(ProcessState::RUNNING, m_clock.getTime());
        m_runningProcess->resetQuantum(); // Reset Quantum on restore

        emit processStateChanged(m_runningProcess->getPid(), ProcessState::RUNNING);
        emit logMessage(QString("Context Switch complete. Running P%1").arg(m_runningProcess->getPid()));
      }
    } else {
      // CPU is free for user process
      handleCpuExecution();

      // Only schedule if we are not currently switching and have no running process
      if (m_runningProcess == nullptr && m_contextSwitchCounter == 0) handleScheduling();
    }

    m_clock.tick();

    // Check for termination condition
    // Simplificado: si no hay procesos activos, parar
    if (m_incomingProcesses.empty() && m_blockedQueue.empty() && 
      m_memoryWaitQueue.empty() && m_runningProcess == nullptr &&
      m_nextProcess == nullptr && !m_scheduler->hasReadyProcesses()) {
       // O verificar si todos los m_processes están TERMINATED
       bool allTerminated = true;
       for(const auto& p : m_processes) {
         if (p->getState() != ProcessState::TERMINATED) {
           allTerminated = false;
           break;
         }
       }
       if (allTerminated) {
         m_isRunning = false;
         emit simulationFinished();
         emit logMessage("All processes finished.");
       }
    }
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
          emit logMessage(QString("Preemption: P%1 (Prio %2) displaces P%3 (Prio %4)")
              .arg(p->getPid()).arg(p->getPriority())
              .arg(current->getPid()).arg(current->getPriority()));

          triggerContextSwitch(current, nullptr); // Put current back to ready
          // The scheduler will pick the new high-priority process in handleScheduling
        }

        it = m_incomingProcesses.erase(it);
        emit logMessage(QString("Process P%1 arrived.").arg(p->getPid()));
      } else {
        // Como están ordenados, si este no llegó, los siguientes tampoco.
        break;
      }
    }
  }

  void Simulator::handleIO() {
    // IO handling remains simulated in kernel space for simplicity
    // and determinism, even with threaded processes. The thread is sleeping.
    auto it = m_blockedQueue.begin();
    while (it != m_blockedQueue.end()) {
      Process* p = *it;

      // Thread is conceptually blocked. Kernel updates the PCB state.
      // We manually decrement burst duration here because the thread is paused.
      bool burstFinished = p->updateCurrentBurst(1);
      p->addIoTime(1);

      if (burstFinished) {
        p->advanceToNextBurst();

        // Back to READY
        p->setState(ProcessState::READY,m_clock.getTime());
        emit processStateChanged(p->getPid(), ProcessState::READY);
        m_scheduler->addProcess(p);

        // Preemption on IO Completion could also happen here for Priority Scheduling
        // We omit it for simplicity, but it follows the same logic as Arrivals.

        it = m_blockedQueue.erase(it);
        emit logMessage(QString("Process P%1 finished I/O.").arg(p->getPid()));
      } else {
        ++it;
      }
    }
  }

  void Simulator::handlePageFaults() {
    // Kernel simulates disk latency.
    auto it = m_memoryWaitQueue.begin();
    while (it != m_memoryWaitQueue.end()) {
      MemoryWaitInfo& info = *it;
      
      // Simular tiempo de disco para cargar la página
      info.ticksRemaining--;
      info.process->addIoTime(1); // Contamos espera de disco como IO Time

      if (info.ticksRemaining <= 0) {
        // Notificar al MemoryManager que la carga física ha terminado.
        // Esto actualiza la PageTableEntry a 'present = true'.
        m_memoryManager->completePageLoad(info.process->getPid(), info.pageNumber);
        
        info.process->setState(ProcessState::READY, m_clock.getTime());
        emit processStateChanged(info.process->getPid(), ProcessState::READY);
        m_scheduler->addProcess(info.process);
        
        emit logMessage(QString("Process P%1 resolved Page Fault.").arg(info.process->getPid()));
        it = m_memoryWaitQueue.erase(it);
      } else {
        ++it;
      }
    }
  }

  void Simulator::handleCpuExecution() {
    if (!m_runningProcess) return;

    // MMU Check (Hardware Instruction Fetch simulation)
    int pageRequired = m_runningProcess->getCurrentPageRequirement();
    if (!m_memoryManager->isPageLoaded(m_runningProcess->getPid(), pageRequired)) {
      // Page Fault Exception
      emit logMessage(QString("Page Fault during exec: P%1 needs Page %2")
          .arg(m_runningProcess->getPid()).arg(pageRequired));

      m_runningProcess->incrementPageFaults();
      m_memoryManager->requestPage(m_runningProcess->getPid(), pageRequired);

      m_runningProcess->setState(ProcessState::WAITING_MEMORY, m_clock.getTime());
      emit processStateChanged(m_runningProcess->getPid(), ProcessState::WAITING_MEMORY);

      m_memoryWaitQueue.push_back({m_runningProcess, PAGE_FAULT_PENALTY, pageRequired});
      m_runningProcess = nullptr; // Immediate yield on fault
      return; // Tick used for the faulting instruction attempt
    }

    // ORCHESTRATION: Wake up the thread
    // Instead of calling updateCurrentBurst directly, we signal the thread.
    m_runningProcess->signalRun();

    // BARRIER: Wait for the thread to finish its tick logic
    // This ensures the thread actually executed its instruction cycle.
    m_runningProcess->waitForTickCompletion();

    // Post-Execution Kernel Accounting
    m_runningProcess->addCpuTime(1);
    m_runningProcess->advanceInstructionPointer();
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
        emit logMessage(QString("Process P%1 Terminated.").arg(m_runningProcess->getPid()));

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
        emit logMessage(QString("Quantum expired for P%1").arg(m_runningProcess->getPid()));
        m_runningProcess->incrementPreemptions();
        triggerContextSwitch(m_runningProcess, nullptr);
      }
    }
  }

  void Simulator::handleScheduling() {
    if (!m_scheduler->hasReadyProcesses()) return;
    Process* candidate = m_scheduler->getNextProcess();
    if (!candidate) return;

    // Initiate Context Switch
    m_nextProcess = candidate;
    m_contextSwitchCounter = CONTEXT_SWITCH_DURATION;

    // We do NOT set state to RUNNING yet.
    // It remains READY until CS is done in step().
    emit logMessage(QString("Scheduler selected P%1. Switching context...").arg(candidate->getPid()));
  }

  void Simulator::triggerContextSwitch(Process* current, Process* next) {
    if (current) {
      current->setState(ProcessState::READY, m_clock.getTime());
      emit processStateChanged(current->getPid(), ProcessState::READY);
      m_scheduler->addProcess(current);
    }
    m_runningProcess = nullptr;

    // If we have a specific next process (direct switch), set it up
    // Otherwise, set running to null so handleScheduling picks one
    if (next) {
      m_nextProcess = next;
      m_contextSwitchCounter = CONTEXT_SWITCH_DURATION;
    }
    // If next is null, handleScheduling will run next tick or same tick if called before
  }
}
