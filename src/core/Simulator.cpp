#include "waos/core/Simulator.h"
#include "waos/core/Parser.h"
#include <iostream>
#include <algorithm>

namespace waos::core {

  Simulator::Simulator(QObject* parent)
    : QObject(parent),
      m_runningProcess(nullptr),
      m_isRunning(false) {
  }

  Simulator::~Simulator() = default;

  bool Simulator::loadProcesses(const std::string& filePath) {
    try {
      auto processInfos = Parser::parseFile(filePath);

      // Clear existing data
      m_processes.clear();
      m_incomingProcesses.clear();
      m_blockedQueue.clear();
      m_memoryWaitQueue.clear();
      m_runningProcess = nullptr;

      // Convert ProcessInfo (DTO) to Process (Entity)
      for (const auto& info : processInfos) {
        auto process = std::make_unique<Process>(
          info.pid,
          info.arrivalTime,
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

  void Simulator::setScheduler(std::unique_ptr<IScheduler> scheduler) {
    m_scheduler = std::move(scheduler);
  }

  void Simulator::setMemoryManager(std::unique_ptr<IMemoryManager> memoryManager) {
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

    // We will launch the loop here (likely in a thread).
    // For now, this sets the state.
  }

  void Simulator::stop() {
    m_isRunning = false;
    emit logMessage("Simulation stopped.");
  }

  void Simulator::reset() {
    stop();
    m_runningProcess = nullptr;
    m_blockedQueue.clear();
    m_memoryWaitQueue.clear(); 
    // Reset logic will be refined here (reset clock, process states, etc.)
    // m_clock.reset(); 
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

    handleArrivals();
    handleIO();
    handlePageFaults();
    handleCpuExecution();
    handleScheduling();

    m_clock.tick();

    // Check for termination condition
    // Simplificado: si no hay procesos activos, parar
    if (m_incomingProcesses.empty() && m_blockedQueue.empty() && 
      m_memoryWaitQueue.empty() &&
      m_runningProcess == nullptr && !m_scheduler->hasReadyProcesses()) {
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
        // Attempt to reserve initial structures
        m_memoryManager->allocateForProcess(p->getPid(), p->getRequiredPages());

        // Move to READY (Scheduler se encarga de la cola)
        p->setState(ProcessState::READY, now);
        emit processStateChanged(p->getPid(), ProcessState::READY);

        // Aquí delegamos al Scheduler la gestión de la cola de ready
        // Como el Simulator es dueño, pasamos referencia o raw ptr.
        m_scheduler->addProcess(p);

        it = m_incomingProcesses.erase(it);
        emit logMessage(QString("Process P%1 arrived.").arg(p->getPid()));
      } else {
        // Como están ordenados, si este no llegó, los siguientes tampoco.
        break;
      }
    }
  }

  void Simulator::handleIO() {
    auto it = m_blockedQueue.begin();
    while (it != m_blockedQueue.end()) {
      Process* p = *it;

      // Decrementamos la duración del current IO burst
      // Asumimos que 1 tick = 1 burst unit
      bool burstFinished = p->updateCurrentBurst(1);
      p->addIoTime(1);

      if (burstFinished) {
        p->advanceToNextBurst();

        // Back to READY
        uint64_t now = m_clock.getTime();
        p->setState(ProcessState::READY, now);
        emit processStateChanged(p->getPid(), ProcessState::READY);

        m_scheduler->addProcess(p);

        it = m_blockedQueue.erase(it);
        emit logMessage(QString("Process P%1 finished I/O.").arg(p->getPid()));
      } else {
        ++it;
      }
    }
  }

  void Simulator::handlePageFaults() {
    auto it = m_memoryWaitQueue.begin();
    while (it != m_memoryWaitQueue.end()) {
      MemoryWaitInfo& info = *it;
      
      // Simular tiempo de disco para cargar la página
      info.ticksRemaining--;
      info.process->addIoTime(1); // Contamos espera de disco como IO Time

      if (info.ticksRemaining <= 0) {
        // La página ha sido "cargada" (simulado por el tiempo transcurrido)
        // El MemoryManager ya hizo su lógica en requestPage(), aquí solo volvemos a Ready.
        
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
    if (m_runningProcess) {
      // Process "uses" the CPU
      bool burstFinished = m_runningProcess->updateCurrentBurst(1);
      m_runningProcess->addCpuTime(1);

      // Advance Instruction Pointer
      m_runningProcess->advanceInstructionPointer();

      if (burstFinished) {
        m_runningProcess->advanceToNextBurst();

        if (!m_runningProcess->hasMoreBursts()) {
          m_runningProcess->setState(ProcessState::TERMINATED, m_clock.getTime());
          emit processStateChanged(m_runningProcess->getPid(), ProcessState::TERMINATED);
          emit logMessage(QString("Process P%1 Terminated.").arg(m_runningProcess->getPid()));
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
            // El Scheduler decide. Devolvemos a Ready para que el Scheduler re-evalúe.
            m_runningProcess->setState(ProcessState::READY, m_clock.getTime());
            emit processStateChanged(m_runningProcess->getPid(), ProcessState::READY);
            m_scheduler->addProcess(m_runningProcess);
            m_runningProcess = nullptr;
          }
        }
      }
    }
  }

  void Simulator::handleScheduling() {
    if (m_runningProcess == nullptr) {
      Process* candidate = m_scheduler->getNextProcess();
      
      if (candidate) {
        int pageRequired = candidate->getCurrentPageRequirement();

        if (m_memoryManager->isPageLoaded(candidate->getPid(), pageRequired)) {
          m_runningProcess = candidate;
          m_runningProcess->setState(ProcessState::RUNNING, m_clock.getTime());
          emit processStateChanged(m_runningProcess->getPid(), ProcessState::RUNNING);
        } else {
          emit logMessage(QString("Page Fault: P%1 needs Page %2").arg(candidate->getPid()).arg(pageRequired));
          
          candidate->incrementPageFaults();

          m_memoryManager->requestPage(candidate->getPid(), pageRequired);

          candidate->setState(ProcessState::WAITING_MEMORY, m_clock.getTime());
          emit processStateChanged(candidate->getPid(), ProcessState::WAITING_MEMORY);
          
          m_memoryWaitQueue.push_back({candidate, PAGE_FAULT_PENALTY});
          
          // No asignamos m_runningProcess. 
          // En el siguiente tick, el scheduler probará con otro proceso si hay alguno disponible.
        }
      }
    }
  }

}
