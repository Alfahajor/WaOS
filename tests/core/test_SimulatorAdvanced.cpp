#include "waos/core/Simulator.h"
#include "waos/core/Process.h"
#include "tests/core/CoreMocks.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <cmath>

using namespace waos::core;

void createTempFile(const std::string& fname, const std::string& content) {
  std::ofstream out(fname);
  out << content;
  out.close();
}

// TEST 1: Validación Matemática de Métricas
void test_metrics_calculation() {
  std::cout << "[RUNNING] test_metrics_calculation..." << std::endl;
  std::string fname = "test_metrics.txt";

  // P1: Llega 0, CPU 2 (Prio 1)
  // P2: Llega 0, CPU 2 (Prio 1)
  // MockScheduler es FIFO, así que P1 ejecuta primero, luego P2.
  createTempFile(fname, 
    "P1 0 CPU(2) 1 1\n"
    "P2 0 CPU(2) 1 1\n"
  );

  Simulator sim;
  sim.loadProcesses(fname);

  auto sched = std::make_unique<MockScheduler>();
  auto mem = std::make_unique<MockMemoryManager>();
  mem->everythingLoaded = true;

  sim.setScheduler(std::move(sched));
  sim.setMemoryManager(std::move(mem));

  sim.start();

  // Ejecución Determinista Esperada:
  // T0 (Inicio): Llegada P1, P2. Selección P1 (Inmediata). P1 Running.
  // T1: P1 ejecuta (1/2).
  // T2: P1 ejecuta (2/2). P1 Termina. Selección P2 (Inmediata). P2 Running.
  //     (P1 FinishTime = 2, Turnaround = 2, Wait = 0).
  // T3: P2 ejecuta (1/2).
  // T4: P2 ejecuta (2/2). P2 Termina.
  //     (P2 FinishTime = 4, Turnaround = 4, Wait = 2).

  // Avg Turnaround = (2 + 4) / 2 = 3.0
  // Avg Wait = (0 + 2) / 2 = 1.0

  int maxTicks = 20;
  while(sim.isRunning() && maxTicks-- > 0) {
    sim.tick();
  }

  auto metrics = sim.getSimulatorMetrics();

  assert(metrics.completedProcesses == 2);

  std::cout << "  -> Avg Wait: " << metrics.avgWaitTime << std::endl;
  std::cout << "  -> Avg Turnaround: " << metrics.avgTurnaroundTime << std::endl;

  // Tolerancia pequeña para floats
  assert(std::abs(metrics.avgTurnaroundTime - 3.0) < 0.1);
  assert(std::abs(metrics.avgWaitTime - 1.0) < 0.1);

  std::cout << "[PASSED] test_metrics_calculation" << std::endl;
  std::remove(fname.c_str());
}

// TEST 2: Apropiación (Preemption) por Prioridad
void test_preemption_logic() {
  std::cout << "[RUNNING] test_preemption_logic..." << std::endl;
  std::string fname = "test_preempt.txt";

  // P1: Prioridad 2 (Peor), llega en 0, muy largo.
  // P2: Prioridad 1 (Mejor), llega en 2.
  createTempFile(fname, 
    "P1 0 CPU(10) 2 1\n"
    "P2 2 CPU(2) 1 1\n"
  );

  Simulator sim;
  sim.loadProcesses(fname);

  auto sched = std::make_unique<MockScheduler>();
  auto mem = std::make_unique<MockMemoryManager>();
  mem->everythingLoaded = true;

  sim.setScheduler(std::move(sched));
  sim.setMemoryManager(std::move(mem));
  sim.start();

  // Tick 0: Llega P1. Se asigna inmediatamente
  sim.tick();
  assert(sim.getRunningProcess() != nullptr);
  assert(sim.getRunningProcess()->getPid() == 1);

  // Tick 1: P1 se ejecuta
  sim.tick();
  assert(sim.getRunningProcess()->getPid() == 1);

  // Tick 2: 
  // handleCpuExecution: P1 Ejecuta su tick
  // handleArrivals: Llega P2 (Prio 1 < Prio 2).
  // Preemption activada: triggerContextSwitch(P1, P2). Overhead iniciado.
  // Estado: runningProcess = nullptr, next = P2.
  sim.tick(); 
  assert(sim.getRunningProcess() == nullptr); // Está en overhead de CS

  // Tick 3: CS Overhead consume su tick.
  // Al final del tick, el contador llega a 0 y P2 entra a running.
  sim.tick();

  // Tick 4: P2 debe estar corriendo.
  sim.tick();

  auto* running = sim.getRunningProcess();
  assert(running != nullptr);
  assert(running->getPid() == 2); // P2 desplazó a P1.

  std::cout << "[PASSED] test_preemption_logic" << std::endl;
  std::remove(fname.c_str());
}

// TEST 3: Sin penalización al inicio
void test_no_context_switch_overhead_on_start() {
  std::cout << "[RUNNING] test_no_context_switch_overhead_on_start..." << std::endl;
  std::string fname = "test_start_overhead.txt";

  // P1: CPU(3).
  createTempFile(fname, "P1 0 CPU(3) 1 1\n");

  Simulator sim;
  sim.loadProcesses(fname);

  auto sched = std::make_unique<MockScheduler>();
  auto mem = std::make_unique<MockMemoryManager>();
  mem->everythingLoaded = true;

  sim.setScheduler(std::move(sched));
  sim.setMemoryManager(std::move(mem));
  sim.start();

  // T0: Llega P1. handleScheduling lo selecciona inmediatamente (Fix P1).
  sim.tick(); 
  
  // Verificación: Debe estar corriendo YA, sin ticks de espera.
  assert(sim.getRunningProcess() != nullptr);
  assert(sim.getRunningProcess()->getPid() == 1);

  // Ejecutar restante (T1, T2, T3 termina).
  sim.tick();
  sim.tick();
  sim.tick();

  assert(sim.getRunningProcess() == nullptr);
  
  // Verificar métrica de CS
  auto metrics = sim.getSimulatorMetrics();
  assert(metrics.totalContextSwitches == 0); 
  assert(metrics.completedProcesses == 1);

  std::cout << "[PASSED] test_no_context_switch_overhead_on_start" << std::endl;
  std::remove(fname.c_str());
}

// TEST 4: Sin penalización al terminar (Fix P1)
void test_no_cs_overhead_on_termination() {
  std::cout << "[RUNNING] test_no_cs_overhead_on_termination..." << std::endl;
  std::string fname = "test_term_overhead.txt";

  // P1 y P2 llegan juntos. FIFO. P1(1), P2(1).
  createTempFile(fname, 
    "P1 0 CPU(1) 1 1\n"
    "P2 0 CPU(1) 1 1\n"
  );

  Simulator sim;
  sim.loadProcesses(fname);
  
  auto sched = std::make_unique<MockScheduler>();
  auto mem = std::make_unique<MockMemoryManager>();
  mem->everythingLoaded = true;

  sim.setScheduler(std::move(sched));
  sim.setMemoryManager(std::move(mem));
  sim.start();

  // T0: Llega P1, P2. P1 seleccionado inmediato.
  sim.tick();
  assert(sim.getRunningProcess()->getPid() == 1);

  // T1: P1 ejecuta (1/1) y termina.
  // handleScheduling detecta CPU libre y selecciona P2 inmediato (Fix P1).
  sim.tick();
  
  // Verificación: P2 debe estar corriendo inmediatamente en el siguiente tick disponible,
  // sin ticks vacíos intermedios.
  assert(sim.getRunningProcess() != nullptr);
  assert(sim.getRunningProcess()->getPid() == 2);

  // Verificar métrica de CS
  auto metrics = sim.getSimulatorMetrics();
  assert(metrics.totalContextSwitches == 0);

  std::cout << "[PASSED] test_no_cs_overhead_on_termination" << std::endl;
  std::remove(fname.c_str());
}

// TEST 5: Determinismo de Referencias de Memoria
void test_memory_reference_determinism() {
  std::cout << "[RUNNING] test_memory_reference_determinism..." << std::endl;

  std::queue<Burst> bursts;
  bursts.push({BurstType::CPU, 50});

  // Crear dos procesos idénticos (mismo PID)
  Process p1(100, 0, 1, bursts, 10);
  Process p2(100, 0, 1, bursts, 10);

  // Iniciar hilos para que generen (aunque generateReferenceString ocurre en ctor)
  // Verificar que la secuencia de páginas requerida es idéntica
  const auto& refs1 = p1.getPageReferenceString();
  const auto& refs2 = p2.getPageReferenceString();

  assert(refs1.size() == refs2.size());
  assert(refs1.size() > 0);

  for(size_t i=0; i < refs1.size(); ++i) {
    assert(refs1[i] == refs2[i]);
  }

  // Verificar que PIDs distintos generan secuencias distintas (probabilístico pero muy probable)
  Process p3(101, 0, 1, bursts, 10);
  const auto& refs3 = p3.getPageReferenceString();

  bool different = false;
  for(size_t i=0; i < std::min(refs1.size(), refs3.size()); ++i) {
    if(refs1[i] != refs3[i]) {
      different = true;
      break;
    }
  }
  assert(different);

  std::cout << "[PASSED] test_memory_reference_determinism" << std::endl;
}

int main() {
  test_metrics_calculation();
  test_preemption_logic();
  test_no_context_switch_overhead_on_start();
  test_no_cs_overhead_on_termination();
  test_memory_reference_determinism();

  return 0;
}
