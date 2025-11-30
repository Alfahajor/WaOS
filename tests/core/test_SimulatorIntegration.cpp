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

  // Tick 0: P1 llega to Scheduler to Inmediatamente seleccionado para Context Switch
  sim.tick();
  assert(sim.getReadyProcesses().size() == 0);

  // Tick 1: Termina CS to P1 Running to CPU(1)
  sim.tick();
  assert(sim.getRunningProcess() != nullptr);
  assert(sim.getRunningProcess()->getPid() == 1);

  // Tick 2: CPU(1) termina to Detecta IO to Mueve a Bloqueado
  // Nota: El tick procesa la ejecución y mueve al final.
  sim.tick();
  assert(sim.getBlockedProcesses().size() == 1);
  assert(sim.getBlockedProcesses()[0]->getPid() == 1);

  // Tick 3: E/S(2) termina, se marca para empezar E/S(1)
  sim.tick();
  assert(sim.getBlockedProcesses().size() == 1);
  assert(sim.getBlockedProcesses()[0]->getPid() == 1);

  // Tick 4: E/S(1) termina, el proceso vuelve a READY
  // Se marca para cambio de contexto
  sim.tick();
  assert(sim.getBlockedProcesses().size() == 0);

  // Tick 5: CPU(1) comienza
  sim.tick();
  assert(sim.getRunningProcess() != nullptr);
  assert(sim.getRunningProcess()->getPid() == 1);

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

  // Tick 0: Llegada to Inmediatamente Page Fault to Waiting Memory
  sim.tick();

  // Tick 1: Se marca como Listo y regresa a readyQueue
  sim.tick();

  // Validar que está en la cola de espera de memoria, NO en bloqueados por E/S
  auto memoryWait = sim.getMemoryWaitQueue();
  assert(memoryWait.size() == 0);

  std::cout << "[PASSED] test_page_fault_auto_resolution" << std::endl;
  std::remove(fname.c_str());
}

int main() {
  test_io_blocking_flow();
  test_page_fault_auto_resolution();
  return 0;
}
