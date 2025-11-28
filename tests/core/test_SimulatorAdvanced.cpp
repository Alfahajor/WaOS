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
  // T0: Llegada P1, P2. CS P1 (Overhead).
  // T1: P1 Run (1/2). (Wait P1 = 0)
  // T2: P1 Run (2/2). Fin. (Turnaround P1 = T3 - T0 = 3).
  // T3: CS P2 (Overhead).
  // T4: P2 Run (1/2).
  // T5: P2 Run (2/2). Fin. (Turnaround P2 = T6 - T0 = 6).
  // Wait P2 = T3 (Momento selección) - T0 = 3.

  // Avg TA = (3 + 6) / 2 = 4.5
  // Avg Wait = (0 + 3) / 2 = 1.5

  // Corremos hasta que termine
  int maxTicks = 20;
  while(sim.isRunning() && maxTicks-- > 0) {
    sim.tick();
  }

  auto metrics = sim.getSimulatorMetrics();

  assert(metrics.completedProcesses == 2);

  // Verificamos rangos razonables dado el overhead
  // Avg Turnaround = (3 + 6) / 2 = 4.5
  // Avg Wait = (1 + 4) / 2 = 2.5

  std::cout << "  -> Avg Wait: " << metrics.avgWaitTime << std::endl;
  std::cout << "  -> Avg Turnaround: " << metrics.avgTurnaroundTime << std::endl;

  // Tolerancia pequeña para floats
  assert(std::abs(metrics.avgTurnaroundTime - 4.5) < 0.1);
  assert(std::abs(metrics.avgWaitTime - 2.5) < 0.1);

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

  // Tick 0: Llega P1.
  sim.tick();

  // Tick 1: CS a P1.
  sim.tick();

  // Tick 2: P1 Ejecuta. Llega P2 (Prio 1).
  // Aquí handleArrivals detecta que P2(1) < P1(2). Debe disparar Preemption.
  sim.tick(); 

  // Estado esperado: P1 debe haber sido devuelto a Ready. CS debe estar ocurriendo para P2.
  // Como hubo CS, runningProcess es null o P2 dependiendo de la lógica exacta de CS instantáneo vs diferido.
  // En Simulator.cpp: triggerContextSwitch pone running=null, y next=P2.

  assert(sim.getRunningProcess() == nullptr); // Está en CS

  // Tick 3: CS Overhead (cambiando a P2).
  sim.tick();

  // Tick 4: P2 debe estar corriendo.
  sim.tick();

  auto* running = sim.getRunningProcess();
  assert(running != nullptr);
  assert(running->getPid() == 2); // ¡ÉXITO! P2 desplazó a P1.

  std::cout << "[PASSED] test_preemption_logic" << std::endl;
  std::remove(fname.c_str());
}

// TEST 3: Determinismo de Referencias de Memoria
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
  test_memory_reference_determinism();
  return 0;
}
