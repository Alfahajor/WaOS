#include "waos/core/Simulator.h"
#include "waos/core/Process.h"
#include "tests/core/CoreMocks.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace waos::core;

void createProcessFile(const std::string& filename) {
  std::ofstream out(filename);
  // P1: Page Fault al inicio (Mem Dev), luego CPU, luego I/O (E/S Dev).
  // P2: Llega en t=2 para poblar Ready Queue.
  out << "P1 0 CPU(3),E/S(3),CPU(2) 2 1\n"; 
  out << "P2 2 CPU(4) 2 1\n";
  out << "P3 6 CPU(2) 2 1\n";
  out.close();
}

std::string formatProcess(const Process* p) {
  if (!p) return "";
  std::stringstream ss;
  ss << "P" << p->getPid() << " (" << p->getCurrentBurstDuration() << ")";
  return ss.str();
}

std::string formatMemoryWait(const waos::common::MemoryWaitInfo& info) {
  std::stringstream ss;
  ss << "P" << info.pid << " (" << info.ticksRemaining << ")";
  return ss.str();
}

std::string formatQueue(const std::vector<const Process*>& queue) {
  if (queue.empty()) return "-";
  std::stringstream ss;
  for (size_t i = 0; i < queue.size(); ++i) {
    ss << formatProcess(queue[i]);
    if (i < queue.size() - 1) ss << " ";
  }
  return ss.str();
}

std::string formatWaitQueue(const std::vector<waos::common::MemoryWaitInfo>& queue) {
  if (queue.empty()) return "-";
  std::stringstream ss;
  for (size_t i = 0; i < queue.size(); ++i) {
    ss << formatMemoryWait(queue[i]);
    if (i < queue.size() - 1) ss << " ";
  }
  return ss.str();
}

void run_trace_simulation() {
  std::string filename = "procesos_trace.txt";
  createProcessFile(filename);

  Simulator sim;
  if (!sim.loadProcesses(filename)) {
    std::cerr << "Error loading processes." << std::endl;
    return;
  }

  // Usamos MockScheduler (FIFO) y MockMemory
  auto sched = std::make_unique<MockScheduler>();
  auto mem = std::make_unique<MockMemoryManager>();
  
  // everythingLoaded = false provoca PageFaults en el primer acceso de cada página.
  // Esto permite visualizar la columna "Mem Dev".
  mem->everythingLoaded = false; 

  sim.setScheduler(std::move(sched));
  sim.setMemoryManager(std::move(mem));

  sim.start();

  // Definición de anchos de columna
  const int w_t = 3;
  const int w_cpu = 10;
  const int w_ready = 25;
  const int w_iod = 25;
  const int w_memd = 25;

  // Header
  std::cout << std::left 
            << std::setw(w_t) << "t" << " | "
            << std::setw(w_cpu) << "CPU" << " | "
            << std::setw(w_ready) << "Ready Queue" << " | "
            << std::setw(w_iod) << "E/S Dev" << " | "
            << std::setw(w_memd) << "Mem Dev" 
            << std::endl;

  std::cout << std::setfill('-') 
            << std::setw(w_t) << "" << "-+-"
            << std::setw(w_cpu) << "" << "-+-"
            << std::setw(w_ready) << "" << "-+-"
            << std::setw(w_iod) << "" << "-+-"
            << std::setw(w_memd) << "" 
            << std::setfill(' ') << std::endl;

  // Simulation Loop
  while (sim.isRunning()) {
    // Capturar estado actual (Snapshot del inicio del tick)
    uint64_t t = sim.getCurrentTime();
    
    // Avanzar simulación
    sim.tick();

    const Process* cpuProc = sim.getRunningProcess();
    auto readyQueue = sim.getReadyProcesses();
    auto ioDevs = sim.getBlockedProcesses(); // En este simulador, Blocked = IO Active
    auto memDevs = sim.getMemoryWaitQueue(); // En este simulador, WaitMem = Disk Active

    // Lógica de visualización CPU
    std::string cpuStr = "-";
    if (cpuProc) {
      cpuStr = formatProcess(cpuProc);
    } else {
      // Si no hay CPU, pero hay listos, es Context Switch (C.S)
      // Nota: Con las correcciones recientes (Fix P1), esto solo se verá en Preempción.
      if (!readyQueue.empty()) cpuStr = "C.S";
    }

    std::string readyStr = formatQueue(readyQueue);
    std::string ioDevStr = formatQueue(ioDevs);
    std::string memDevStr = formatWaitQueue(memDevs);

    // Imprimir Fila
    std::cout << std::left 
              << std::setw(w_t) << t << " | "
              << std::setw(w_cpu) << cpuStr << " | "
              << std::setw(w_ready) << readyStr << " | "
              << std::setw(w_iod) << ioDevStr << " | "
              << std::setw(w_memd) << memDevStr 
              << std::endl;

    // Safety break para evitar bucles infinitos en desarrollo
    if (t > 30) break; 
  }

  std::remove(filename.c_str());
}

int main() {
  run_trace_simulation();
  return 0;
}
