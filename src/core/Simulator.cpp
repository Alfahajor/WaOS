#include "waos/core/Simulator.h"
#include "waos/core/Parser.h"
#include <iostream>
#include <algorithm>

namespace waos::core {

  Simulator::Simulator(QObject* parent)
    : QObject(parent),
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
    // Reset logic will be refined here (reset clock, process states, etc.)
    // m_clock.reset(); 
    emit logMessage("Simulation reset.");
  }

  void Simulator::step() {
    // Simulation Loop Logic
  }

}
