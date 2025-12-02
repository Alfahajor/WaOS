#include "waos/core/Simulator.h"
#include "waos/core/Parser.h"
#include <waos/scheduler/IScheduler.h>
#include <waos/memory/IMemoryManager.h>
#include "waos/common/DataStructures.h"

#include <iostream>
#include <algorithm>

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
                  if (a->getArrivalTime() != b->getArrivalTime()) {
                    return a->getArrivalTime() < b->getArrivalTime();
                  }
                  return a->getPid() < b->getPid();
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

    for (auto& process : m_processes) {
      if (process) {
        process->stopThread(); // Joins the thread
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

    // Clear main container (Destructors will run, but threads are already joined)
    m_processes.clear(); 
    m_incomingProcesses.clear();

    emit logMessage("Simulation reset.");
  }

  void Simulator::tick() {
    if (!m_isRunning) return;
    step();
  }

  void Simulator::step() {
    std::lock_guard<std::mutex> lock(m_simulationMutex);

    uint64_t now = m_clock.getTime();
    emit clockTicked(now);

    // Current running process executes its burst for this tick
    if (m_contextSwitchCounter > 0) {
      // Context Switch Overhead, CPU is busy doing kernel work
      m_contextSwitchCounter--;
      if (m_contextSwitchCounter == 0 && m_nextProcess) {
        m_runningProcess = m_nextProcess;
        m_nextProcess = nullptr;

        m_runningProcess->setState(ProcessState::RUNNING, m_clock.getTime());

        emit processStateChanged(m_runningProcess->getPid(), ProcessState::RUNNING);
        emit logMessage(QString("Context Switch complete. Running P%1").arg(m_runningProcess->getPid()));
      }
    } else {
      // CPU is free for user process
      handleCpuExecution();
    }

    // Process Arrivals (May cause Preemption)
    handleArrivals();

    // IO Devices (Parallel to CPU)
    handleIO();

    // Memory Disk Operations (Parallel to CPU)
    handlePageFaults();

    // Scheduling logic runs if CPU is free AND no switch is in progress.
    if (m_runningProcess == nullptr && m_contextSwitchCounter == 0) handleScheduling();

    updateMetrics();
    m_clock.tick();
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
        p->setState(ProcessState::READY,m_clock.getTime());
        emit processStateChanged(p->getPid(), ProcessState::READY);
        m_scheduler->addProcess(p);

        // Preemption on IO Completion could also happen here for Priority Scheduling
        // We omit it for simplicity, but it follows the same logic as Arrivals.

        it = m_blockedQueue.erase(it);
        emit logMessage(QString("Process P%1 finished I/O.").arg(p->getPid()));
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
      if (info.process) info.process->addIoTime(1); // Count disk wait as IO Time

      if (info.ticksRemaining <= 0) {
        // Notify to MemoryManager que la carga física is finished.
        m_memoryManager->completePageLoad(info.process->getPid(), info.pageNumber);
        
        // Reset Quantum on Fault Resolution
        info.process->resetQuantum();

        info.process->setState(ProcessState::READY, m_clock.getTime());
        emit processStateChanged(info.process->getPid(), ProcessState::READY);
        m_scheduler->addProcess(info.process);
        
        emit logMessage(QString("Process P%1 resolved Page Fault.").arg(info.process->getPid()));
        it = m_memoryWaitQueue.erase(it);
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
      m_totalPageFaults++;
      m_memoryManager->requestPage(m_runningProcess->getPid(), pageRequired);

      m_runningProcess->setState(ProcessState::WAITING_MEMORY, m_clock.getTime());
      emit processStateChanged(m_runningProcess->getPid(), ProcessState::WAITING_MEMORY);

      m_memoryWaitQueue.push_back({m_runningProcess, m_pageFaultPenalty, pageRequired});
      m_runningProcess = nullptr; // Immediate yield on fault
      return; // Tick used for the faulting instruction attempt
    }

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

    if (!candidate) {
      emit logMessage("Warning: Scheduler returned null despite reporting ready processes.");
      return;
    }

    // Initiate Context Switch
    m_runningProcess = candidate;
    m_runningProcess->setState(ProcessState::RUNNING, m_clock.getTime());

    emit processStateChanged(m_runningProcess->getPid(), ProcessState::RUNNING);
    emit logMessage(QString("Scheduler selected P%1. Starting immediately (No CS overhead)").arg(candidate->getPid()));
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

    for(const auto& p : m_processes) {
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
    std::lock_guard<std::mutex> lock(m_simulationMutex);
    std::vector<const Process*> result;
    for(const auto& p : m_processes) result.push_back(p.get());
    return result;
  }

  const Process* Simulator::getRunningProcess() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);
    return m_runningProcess;
  }

  std::vector<const Process*> Simulator::getBlockedProcesses() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);
    std::vector<const Process*> result;
    for(const auto* p : m_blockedQueue) result.push_back(p);
    return result;
  }

  std::vector<waos::common::MemoryWaitInfo> Simulator::getMemoryWaitQueue() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);
    std::vector<waos::common::MemoryWaitInfo> result;
    for(const auto& item : m_memoryWaitQueue) {
      result.push_back({item.process->getPid(), item.pageNumber, item.ticksRemaining});
    }
    return result;
  }

  std::vector<const Process*> Simulator::getReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);
    if (m_scheduler) return m_scheduler->peekReadyQueue();
    return {};
  }

  waos::common::SimulatorMetrics Simulator::getSimulatorMetrics() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);
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

}
