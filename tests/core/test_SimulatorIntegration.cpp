#include "waos/core/Simulator.h"
#include "tests/core/CoreMocks.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <cstdio>

using namespace waos::core;

void createTestFile(const std::string& fname, const std::string& content) {
  std::ofstream out(fname);
  out << content;
  out.close();
}

void test_io_blocking_flow() {
  std::cout << "[RUNNING] test_io_blocking_flow..." << std::endl;
  std::string fname = "test_io.txt";

  createTestFile(fname, "P1 0 CPU(1),E/S(2),CPU(1) 1 4\n");

  Simulator sim;
  if (!sim.loadProcesses(fname)) {
    std::cerr << "Failed to load processes" << std::endl;
    assert(false);
  }
  
  auto sched = std::make_unique<MockScheduler>();
  auto mem = std::make_unique<MockMemoryManager>();
  auto memPtr = mem.get();
  
  sim.setScheduler(std::move(sched));
  sim.setMemoryManager(std::move(mem));

  // Evitar Page Faults
  memPtr->everythingLoaded = true;

  sim.start();

  // Tick 0: P1 llega y seleccionado inmediatamente
  sim.tick();
  assert(sim.getRunningProcess() != nullptr);
  assert(sim.getRunningProcess()->getPid() == 1);

  // Tick 1: P1 ejecuta CPU(1). Termina ráfaga. Bloquea para IO.
  // Fin de Tick 1: P1 en BlockedQueue. Running = nullptr.
  sim.tick();
  assert(sim.getBlockedProcesses().size() == 1);
  assert(sim.getBlockedProcesses()[0]->getPid() == 1);
  assert(sim.getRunningProcess() == nullptr);

  // Tick 2: IO(2) a IO(1). Sigue bloqueado
  sim.tick();
  assert(sim.getBlockedProcesses().size() == 1);

  // Tick 3: I/O(1) a I/O(0). Termina IO. Pasa a Ready.
  // handleScheduling ve CPU libre y selecciona P1 inmediatamente.
  // Fin de Tick 3: P1 Running.
  sim.tick();
  assert(sim.getBlockedProcesses().size() == 0);
  assert(sim.getRunningProcess() != nullptr);
  assert(sim.getRunningProcess()->getPid() == 1);

  // Tick 4: CPU(1) comienza y termina. P1 terminado
  sim.tick();
  assert(sim.getRunningProcess() == nullptr);
  assert(sim.getSimulatorMetrics().completedProcesses == 1);

  std::cout << "[PASSED] test_io_blocking_flow" << std::endl;
  std::remove(fname.c_str());
}

void test_page_fault_auto_resolution() {
  std::cout << "[RUNNING] test_page_fault_auto_resolution..." << std::endl;
  std::string fname = "test_pf.txt";

  createTestFile(fname, "P1 0 CPU(10) 1 1\n");

  Simulator sim;
  sim.loadProcesses(fname);

  auto sched = std::make_unique<MockScheduler>();
  auto mem = std::make_unique<MockMemoryManager>();

  sim.setScheduler(std::move(sched));
  sim.setMemoryManager(std::move(mem));

  sim.start();

  // Tick 0: P1 llega, se asigna CPU inmediatamente
  // El proceso queda en estado RUNNING al final del tick.
  sim.tick();
  assert(sim.getRunningProcess() != nullptr);
  assert(sim.getRunningProcess()->getPid() == 1);

  // Tick 1: handleCpuExecution, ocurre Page Fault.
  // El proceso es movido a WAITING_MEMORY y añadido a memoryWaitQueue.
  // Running pasa a nullptr.
  sim.tick();

  // Validar que está en la cola de espera de memoria.
  auto memoryWait = sim.getMemoryWaitQueue();
  auto blocked = sim.getBlockedProcesses();

  // Debe haber 1 proceso esperando memoria
  assert(memoryWait.size() == 1);
  assert(memoryWait[0].pid == 1);
  
  // NO debe estar en la cola de E/S genérica
  assert(blocked.size() == 0);

  // Validar estado del proceso
  auto allProcs = sim.getAllProcesses();
  assert(allProcs[0]->getState() == ProcessState::WAITING_MEMORY);

  std::cout << "[PASSED] test_page_fault_auto_resolution" << std::endl;
  std::remove(fname.c_str());
}

int main() {
  test_io_blocking_flow();
  test_page_fault_auto_resolution();
  return 0;
}
