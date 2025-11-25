/**
 * @file MockDataProvider.cpp
 * @brief Implementación del proveedor de datos mock
 */

#include "mock/MockDataProvider.h"

#include <chrono>
#include <queue>

namespace waos::gui::mock {

std::mt19937& MockDataProvider::getRNG() {
  static std::mt19937 rng(static_cast<unsigned>(
      std::chrono::steady_clock::now().time_since_epoch().count()));
  return rng;
}

int MockDataProvider::randomInt(int min, int max) {
  std::uniform_int_distribution<int> dist(min, max);
  return dist(getRNG());
}

double MockDataProvider::randomDouble(double min, double max) {
  std::uniform_real_distribution<double> dist(min, max);
  return dist(getRNG());
}

std::vector<std::unique_ptr<waos::core::Process>> MockDataProvider::generateProcessList(uint64_t tick) {
  std::vector<std::unique_ptr<waos::core::Process>> processes;

  // Generar 5-8 procesos con estados variados
  int processCount = randomInt(5, 8);

  for (int i = 1; i <= processCount; ++i) {
    // Crear bursts aleatorios
    std::queue<waos::core::Burst> bursts;
    int burstCount = randomInt(2, 5);

    for (int j = 0; j < burstCount; ++j) {
      waos::core::BurstType type = (j % 2 == 0) ? waos::core::BurstType::CPU : waos::core::BurstType::IO;
      int duration = randomInt(2, 10);
      bursts.push({type, duration});
    }

    auto process = std::make_unique<waos::core::Process>(
        i,                  // pid
        randomInt(0, 10),   // arrivalTime
        randomInt(0, 3),    // priority
        std::move(bursts),  // bursts
        randomInt(3, 10)    // requiredPages
    );

    // Asignar estado según probabilidad
    int stateRoll = randomInt(1, 100);
    waos::core::ProcessState state;

    if (stateRoll <= 15) {
      state = waos::core::ProcessState::RUNNING;
    } else if (stateRoll <= 50) {
      state = waos::core::ProcessState::READY;
    } else if (stateRoll <= 70) {
      state = waos::core::ProcessState::BLOCKED;
    } else if (stateRoll <= 85) {
      state = waos::core::ProcessState::WAITING_MEMORY;
    } else {
      state = waos::core::ProcessState::TERMINATED;
    }

    process->setState(state, tick);

    // Simular estadísticas
    if (state == waos::core::ProcessState::TERMINATED) {
      process->addCpuTime(randomInt(20, 100));
      process->addIoTime(randomInt(10, 50));
      for (int pf = 0; pf < randomInt(0, 5); ++pf) {
        process->incrementPageFaults();
      }
    }

    processes.push_back(std::move(process));
  }

  return processes;
}

waos::common::SimulatorMetrics MockDataProvider::generateSimulatorMetrics(uint64_t tick) {
  waos::common::SimulatorMetrics metrics;

  metrics.currentTick = tick;
  metrics.totalProcesses = randomInt(5, 8);
  metrics.completedProcesses = std::min(static_cast<int>(tick / 20), metrics.totalProcesses);

  if (metrics.completedProcesses > 0) {
    metrics.avgWaitTime = randomDouble(10.0, 50.0);
    metrics.avgTurnaroundTime = randomDouble(50.0, 150.0);
  }

  metrics.cpuUtilization = randomDouble(0.5, 0.95);
  metrics.totalContextSwitches = static_cast<int>(tick / 4);
  metrics.totalPageFaults = static_cast<int>(tick / 3);

  return metrics;
}

std::vector<waos::common::FrameInfo> MockDataProvider::generateFrameStatus(int totalFrames, uint64_t tick) {
  std::vector<waos::common::FrameInfo> frames;
  frames.reserve(totalFrames);

  for (int i = 0; i < totalFrames; ++i) {
    waos::common::FrameInfo frame;
    frame.frameId = i;
    frame.isOccupied = randomInt(0, 100) < 70;  // 70% probabilidad de estar ocupado

    if (frame.isOccupied) {
      frame.ownerPid = randomInt(1, 5);
      frame.pageNumber = randomInt(0, 9);
      frame.loadedAtTick = randomInt(0, static_cast<int>(tick));
    } else {
      frame.ownerPid = -1;
      frame.pageNumber = -1;
      frame.loadedAtTick = 0;
    }

    frames.push_back(frame);
  }

  return frames;
}

std::vector<waos::common::PageTableEntryInfo> MockDataProvider::generatePageTable(int pid, int pages, uint64_t tick) {
  std::vector<waos::common::PageTableEntryInfo> pageTable;
  pageTable.reserve(pages);

  for (int i = 0; i < pages; ++i) {
    waos::common::PageTableEntryInfo entry;
    entry.pageNumber = i;
    entry.present = randomInt(0, 100) < 60;  // 60% probabilidad de estar en memoria

    if (entry.present) {
      entry.frameNumber = randomInt(0, 15);
      entry.referenced = randomInt(0, 100) < 50;
      entry.modified = randomInt(0, 100) < 30;
    } else {
      entry.frameNumber = -1;
      entry.referenced = false;
      entry.modified = false;
    }

    pageTable.push_back(entry);
  }

  return pageTable;
}

waos::common::MemoryStats MockDataProvider::generateMemoryStats(uint64_t tick) {
  waos::common::MemoryStats stats;

  stats.totalFrames = 16;
  stats.usedFrames = randomInt(8, 15);
  stats.totalPageFaults = static_cast<int>(tick / 3);
  stats.totalReplacements = static_cast<int>(tick / 5);
  stats.hitRatio = randomDouble(0.6, 0.9);

  // Generar faults por proceso
  for (int pid = 1; pid <= 5; ++pid) {
    stats.faultsPerProcess[pid] = randomInt(0, 10);
  }

  return stats;
}

waos::common::SchedulerMetrics MockDataProvider::generateSchedulerMetrics(uint64_t tick) {
  waos::common::SchedulerMetrics metrics;

  metrics.totalSchedulingDecisions = static_cast<int>(tick / 2);
  metrics.totalPreemptions = static_cast<int>(tick / 6);

  // Generar conteo de selecciones por proceso
  for (int pid = 1; pid <= 5; ++pid) {
    metrics.selectionCount[pid] = randomInt(5, 20);
  }

  return metrics;
}

}  // namespace waos::gui::mock
