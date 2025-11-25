# Backend API Requirements for GUI Integration

**Documento técnico para equipos Core, Scheduler y Memory**

Este documento especifica las APIs que deben implementarse en los módulos del simulador para permitir la integración con la GUI Qt Quick. Todas las APIs son **read-only** (observadoras) y no modifican el estado de la simulación.

---

## 📋 Índice

1. [Estructuras de Datos Compartidas](#estructuras-de-datos-compartidas)
2. [APIs Requeridas en `Simulator` (Equipo Core)](#apis-requeridas-en-simulator-equipo-core)
3. [APIs Requeridas en `IScheduler` (Equipo Scheduler)](#apis-requeridas-en-ischeduler-equipo-scheduler)
4. [APIs Requeridas en `IMemoryManager` (Equipo Memory)](#apis-requeridas-en-imemorymanager-equipo-memory)
5. [Ejemplo de Consumo desde GUI](#ejemplo-de-consumo-desde-gui)
6. [Prioridad de Implementación](#prioridad-de-implementación)

---

## 1. Estructuras de Datos Compartidas

Crear archivo `include/waos/common/DataStructures.h` con las siguientes definiciones:

```cpp
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace waos::common {

  /**
   * @brief Métricas globales del simulador calculadas en cada tick.
   *
   * IMPORTANTE: Estos valores deben actualizarse automáticamente en cada tick
   * del simulador, no calcularlos bajo demanda.
   */
  struct SimulatorMetrics {
    uint64_t currentTick = 0;              // Tick actual del reloj
    double avgWaitTime = 0.0;              // Tiempo promedio de espera (procesos terminados)
    double avgTurnaroundTime = 0.0;        // Tiempo promedio de retorno (procesos terminados)
    double cpuUtilization = 0.0;           // % de ticks con CPU ocupada
    int totalContextSwitches = 0;          // Cambios de contexto acumulados
    int totalPageFaults = 0;               // Page faults acumulados de todos los procesos
    int completedProcesses = 0;            // Procesos en estado TERMINATED
    int totalProcesses = 0;                // Total de procesos cargados
  };

  /**
   * @brief Información de un frame de memoria física.
   *
   * Retornado por IMemoryManager::getFrameStatus()
   */
  struct FrameInfo {
    int frameId;                           // ID del frame (0 a N-1)
    bool isOccupied;                       // true si contiene una página
    int ownerPid;                          // PID del proceso dueño (-1 si libre)
    int pageNumber;                        // Número de página virtual (-1 si libre)
    uint64_t loadedAtTick;                 // Tick cuando se cargó (0 si nunca)
  };

  /**
   * @brief Entrada de tabla de páginas de un proceso.
   *
   * Retornado por IMemoryManager::getPageTableForProcess()
   */
  struct PageTableEntryInfo {
    int pageNumber;                        // Número de página virtual
    int frameNumber;                       // Frame asignado (-1 si no presente)
    bool present;                          // true si está en memoria física
    bool referenced;                       // Bit de referencia (para LRU)
    bool modified;                         // Bit de modificación (dirty bit)
  };

  /**
   * @brief Estadísticas del gestor de memoria.
   *
   * Retornado por IMemoryManager::getMemoryStats()
   */
  struct MemoryStats {
    int totalFrames;                       // Total de frames en memoria física
    int usedFrames;                        // Frames actualmente ocupados
    int totalPageFaults;                   // Page faults acumulados
    int totalReplacements;                 // Reemplazos de página realizados
    double hitRatio;                       // Ratio de hits (0.0 - 1.0)
    std::map<int, int> faultsPerProcess;   // Page faults por PID
  };

  /**
   * @brief Métricas del planificador.
   *
   * Retornado por IScheduler::getSchedulerMetrics()
   */
  struct SchedulerMetrics {
    int totalSchedulingDecisions;          // Veces que se llamó getNextProcess()
    int totalPreemptions;                  // Preempciones por quantum o prioridad
    std::map<int, int> selectionCount;     // Veces que cada PID fue seleccionado
  };

}
```

---

## 2. APIs Requeridas en `Simulator` (Equipo Core)

### 2.1 Métodos de Observación de Procesos

Agregar a `include/waos/core/Simulator.h` en sección `public`:

```cpp
/**
 * @brief Obtiene lista de todos los procesos cargados en la simulación.
 * @return Vector de punteros const a procesos (NO transferir ownership).
 *
 * Thread-safety: Proteger con m_simulationMutex.
 *
 * PRIORIDAD: CRÍTICA - Sin esto la GUI no puede mostrar lista de procesos.
 */
std::vector<const Process*> getAllProcesses() const;

/**
 * @brief Obtiene el proceso actualmente ejecutándose en CPU.
 * @return Puntero const al proceso en RUNNING, nullptr si CPU está idle.
 *
 * Thread-safety: Proteger con m_simulationMutex.
 *
 * PRIORIDAD: CRÍTICA - Necesario para mostrar proceso activo en UI.
 */
const Process* getRunningProcess() const;

/**
 * @brief Obtiene lista de procesos bloqueados por E/S.
 * @return Vector de punteros const a procesos en cola m_blockedQueue.
 *
 * Thread-safety: Proteger con m_simulationMutex.
 *
 * PRIORIDAD: CRÍTICA - Requerido para Panel de Bloqueos.
 */
std::vector<const Process*> getBlockedProcesses() const;

/**
 * @brief Obtiene lista de procesos esperando carga de página (Page Fault).
 * @return Vector de MemoryWaitInfo const.
 *
 * Thread-safety: Proteger con m_simulationMutex.
 *
 * PRIORIDAD: CRÍTICA - Requerido para Panel de Bloqueos (distinguir E/S de PageFault).
 */
std::vector<MemoryWaitInfo> getMemoryWaitQueue() const;

/**
 * @brief Obtiene lista de procesos en cola Ready del scheduler.
 * @return Vector de punteros const a procesos.
 *
 * Implementación: Llamar a m_scheduler->peekReadyQueue()
 *
 * PRIORIDAD: CRÍTICA - Necesario para mostrar cola Ready.
 */
std::vector<const Process*> getReadyProcesses() const;
```

### 2.2 Métodos de Métricas y Configuración

```cpp
/**
 * @brief Obtiene métricas globales del simulador.
 * @return Struct SimulatorMetrics con valores actualizados.
 *
 * IMPORTANTE: Calcular y actualizar métricas en cada tick, no bajo demanda.
 *
 * Cálculos:
 * - avgWaitTime = suma(totalWaitTime de procesos TERMINATED) / completedProcesses
 * - avgTurnaroundTime = suma(finishTime - arrivalTime) / completedProcesses
 * - cpuUtilization = ticksConCPUOcupada / currentTick
 *
 * PRIORIDAD: ALTA - Requerido por especificación de entrega.
 */
waos::common::SimulatorMetrics getSimulatorMetrics() const;

/**
 * @brief Obtiene nombre del algoritmo de planificación activo.
 * @return String con nombre (ej: "FCFS", "Round Robin", "Priority", "SJF").
 *
 * Implementación: Llamar a m_scheduler->getAlgorithmName()
 *
 * PRIORIDAD: MEDIA - Útil para mostrar en UI.
 */
std::string getSchedulerAlgorithmName() const;

/**
 * @brief Obtiene nombre del algoritmo de reemplazo de páginas activo.
 * @return String con nombre (ej: "FIFO", "LRU", "Optimal").
 *
 * Implementación: Llamar a m_memoryManager->getAlgorithmName()
 *
 * PRIORIDAD: MEDIA - Útil para mostrar en UI.
 */
std::string getMemoryAlgorithmName() const;
```

### 2.3 Implementación de Ejemplo

En `src/core/Simulator.cpp`:

```cpp
std::vector<const Process*> Simulator::getAllProcesses() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);

    std::vector<const Process*> result;
    result.reserve(m_processes.size());

    for (const auto& process : m_processes) {
        result.push_back(process.get());
    }

    return result;
}

const Process* Simulator::getRunningProcess() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);
    return m_runningProcess; // Ya es puntero
}

std::vector<const Process*> Simulator::getBlockedProcesses() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);

    std::vector<const Process*> result(m_blockedQueue.begin(), m_blockedQueue.end());
    return result;
}

std::vector<MemoryWaitInfo> Simulator::getMemoryWaitQueue() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);

    std::vector<MemoryWaitInfo> result(m_memoryWaitQueue.begin(), m_memoryWaitQueue.end());
    return result;
}

std::vector<const Process*> Simulator::getReadyProcesses() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);

    if (!m_scheduler) return {};
    return m_scheduler->peekReadyQueue(); // Requiere implementación en IScheduler
}

waos::common::SimulatorMetrics Simulator::getSimulatorMetrics() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);

    // NOTA: Estas métricas deberían calcularse y mantenerse como miembros
    // actualizados en cada tick. Este código es ilustrativo.

    waos::common::SimulatorMetrics metrics;
    metrics.currentTick = m_clock.getTime();
    metrics.totalProcesses = m_processes.size();

    // Calcular estadísticas de procesos terminados
    int completed = 0;
    uint64_t totalWait = 0;
    uint64_t totalTurnaround = 0;
    int totalFaults = 0;

    for (const auto& proc : m_processes) {
        if (proc->getState() == ProcessState::TERMINATED) {
            ++completed;
            const auto& stats = proc->getStats();
            totalWait += stats.totalWaitTime;
            totalTurnaround += (stats.finishTime - proc->getArrivalTime());
            totalFaults += stats.pageFaults;
        }
    }

    metrics.completedProcesses = completed;
    metrics.totalPageFaults = totalFaults;

    if (completed > 0) {
        metrics.avgWaitTime = static_cast<double>(totalWait) / completed;
        metrics.avgTurnaroundTime = static_cast<double>(totalTurnaround) / completed;
    }

    // CPU utilization: ticks con proceso en CPU / ticks totales
    // TODO: Mantener contador m_cpuBusyTicks actualizado en cada tick
    // metrics.cpuUtilization = static_cast<double>(m_cpuBusyTicks) / metrics.currentTick;

    return metrics;
}
```

---

## 3. APIs Requeridas en `IScheduler` (Equipo Scheduler)

### 3.1 Extensión de la Interfaz

Agregar a `include/waos/scheduler/IScheduler.h`:

```cpp
/**
 * @brief Observa la cola de procesos listos SIN removerlos.
 * @return Vector de punteros const a procesos en cola Ready.
 *
 * IMPORTANTE: Este método NO debe modificar el estado interno.
 * NO usar getNextProcess() ya que consume procesos.
 *
 * Thread-safety: Implementaciones deben proteger acceso a cola interna.
 *
 * PRIORIDAD: CRÍTICA - La GUI necesita ver la cola sin alterarla.
 */
virtual std::vector<const waos::core::Process*> peekReadyQueue() const = 0;

/**
 * @brief Obtiene el tamaño de la cola Ready.
 * @return Cantidad de procesos listos para ejecución.
 *
 * PRIORIDAD: MEDIA - Útil para estadísticas.
 */
virtual size_t getReadyQueueSize() const = 0;

/**
 * @brief Obtiene el nombre del algoritmo de planificación.
 * @return String identificando el algoritmo ("FCFS", "SJF", "Round Robin", "Priority").
 *
 * PRIORIDAD: MEDIA - Necesario para mostrar en UI.
 */
virtual std::string getAlgorithmName() const = 0;

/**
 * @brief Obtiene métricas del planificador.
 * @return Struct SchedulerMetrics con estadísticas acumuladas.
 *
 * PRIORIDAD: BAJA - Útil pero no crítico para funcionalidad básica.
 */
virtual waos::common::SchedulerMetrics getSchedulerMetrics() const = 0;
```

### 3.2 Implementación en FCFSScheduler

En `src/scheduler/FCFSScheduler.cpp`:

```cpp
std::vector<const waos::core::Process*> FCFSScheduler::peekReadyQueue() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<const waos::core::Process*> result;

    // IMPORTANTE: NO usar std::queue directamente (no tiene iteradores)
    // Opción 1: Cambiar m_readyQueue de queue a deque
    // Opción 2: Hacer copia temporal

    std::queue<Process*> temp = m_readyQueue;
    while (!temp.empty()) {
        result.push_back(temp.front());
        temp.pop();
    }

    return result;
}

size_t FCFSScheduler::getReadyQueueSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_readyQueue.size();
}

std::string FCFSScheduler::getAlgorithmName() const {
    return "FCFS (First-Come-First-Served)";
}

waos::common::SchedulerMetrics FCFSScheduler::getSchedulerMetrics() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Retornar métricas acumuladas (deben mantenerse como miembros)
    return m_metrics;
}
```

### 3.3 Nota para Implementaciones Específicas

**RRScheduler**: Igual que FCFS pero retornar "Round Robin (Quantum: X)" con valor de quantum.

**PriorityScheduler**: Iterar sobre `std::map<int, std::queue<Process*>>` y agregar procesos en orden de prioridad.

**SJFScheduler**: Si usan priority_queue, hacer copia temporal para peek sin consumir.

---

## 4. APIs Requeridas en `IMemoryManager` (Equipo Memory)

### 4.1 Extensión de la Interfaz

Agregar a `include/waos/memory/IMemoryManager.h`:

```cpp
/**
 * @brief Obtiene el estado de todos los frames de memoria física.
 * @return Vector de FrameInfo, uno por cada frame.
 *
 * IMPORTANTE: Retornar en orden (frame 0, 1, 2, ..., N-1).
 *
 * PRIORIDAD: CRÍTICA - Necesario para visualización de memoria en GUI.
 */
virtual std::vector<waos::common::FrameInfo> getFrameStatus() const = 0;

/**
 * @brief Obtiene la tabla de páginas de un proceso específico.
 * @param processId PID del proceso.
 * @return Vector de PageTableEntryInfo, una entrada por página del proceso.
 *
 * Si el proceso no existe o no tiene tabla de páginas, retornar vector vacío.
 *
 * PRIORIDAD: CRÍTICA - Necesario para mostrar tabla de páginas en GUI.
 */
virtual std::vector<waos::common::PageTableEntryInfo> getPageTableForProcess(int processId) const = 0;

/**
 * @brief Obtiene el total de frames disponibles en memoria física.
 * @return Cantidad de frames configurados.
 *
 * PRIORIDAD: MEDIA - Útil para UI y validaciones.
 */
virtual int getTotalFrames() const = 0;

/**
 * @brief Obtiene estadísticas del gestor de memoria.
 * @return Struct MemoryStats con métricas acumuladas.
 *
 * PRIORIDAD: ALTA - Requerido por especificación de entrega.
 */
virtual waos::common::MemoryStats getMemoryStats() const = 0;

/**
 * @brief Obtiene el nombre del algoritmo de reemplazo de páginas.
 * @return String identificando el algoritmo ("FIFO", "LRU", "Optimal", etc.).
 *
 * PRIORIDAD: MEDIA - Necesario para mostrar en UI.
 */
virtual std::string getAlgorithmName() const = 0;
```

### 4.2 Ejemplo de Implementación (Pseudocódigo)

```cpp
// En FIFOMemoryManager.cpp (o implementación concreta)

std::vector<waos::common::FrameInfo> FIFOMemoryManager::getFrameStatus() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<waos::common::FrameInfo> result;
    result.reserve(m_frames.size());

    for (size_t i = 0; i < m_frames.size(); ++i) {
        const auto& frame = m_frames[i];

        waos::common::FrameInfo info;
        info.frameId = static_cast<int>(i);
        info.isOccupied = frame.isOccupied;
        info.ownerPid = frame.isOccupied ? frame.ownerPid : -1;
        info.pageNumber = frame.isOccupied ? frame.pageNumber : -1;
        info.loadedAtTick = frame.loadedAtTick;

        result.push_back(info);
    }

    return result;
}

std::vector<waos::common::PageTableEntryInfo> FIFOMemoryManager::getPageTableForProcess(int processId) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_pageTables.find(processId);
    if (it == m_pageTables.end()) {
        return {}; // Proceso no existe
    }

    const auto& pageTable = it->second;
    std::vector<waos::common::PageTableEntryInfo> result;

    for (const auto& entry : pageTable.entries) {
        waos::common::PageTableEntryInfo info;
        info.pageNumber = entry.pageNumber;
        info.frameNumber = entry.frameNumber;
        info.present = entry.present;
        info.referenced = entry.referenced;
        info.modified = entry.modified;

        result.push_back(info);
    }

    return result;
}

waos::common::MemoryStats FIFOMemoryManager::getMemoryStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);

    waos::common::MemoryStats stats;
    stats.totalFrames = m_frames.size();
    stats.usedFrames = countUsedFrames();
    stats.totalPageFaults = m_totalPageFaults;
    stats.totalReplacements = m_totalReplacements;

    int totalAccesses = m_totalPageFaults + m_totalHits;
    stats.hitRatio = totalAccesses > 0 ?
        static_cast<double>(m_totalHits) / totalAccesses : 0.0;

    stats.faultsPerProcess = m_faultsPerProcess; // map mantenido internamente

    return stats;
}

std::string FIFOMemoryManager::getAlgorithmName() const {
    return "FIFO (First-In-First-Out)";
}
```

---

## 5. Ejemplo de Consumo desde GUI

### 5.1 En ProcessMonitorViewModel

```cpp
// src/gui/viewmodels/ProcessMonitorViewModel.cpp

void ProcessMonitorViewModel::onClockTicked(uint64_t tick) {
    if (!m_simulator) return;

    // 1. Obtener todos los procesos
    auto allProcesses = m_simulator->getAllProcesses();

    // 2. Limpiar modelo Qt anterior
    qDeleteAll(m_processItems);
    m_processItems.clear();

    // 3. Crear wrappers Qt para cada proceso
    for (const auto* proc : allProcesses) {
        auto* item = new ProcessItemModel(this);

        item->setPid(proc->getPid());
        item->setState(processStateToString(proc->getState()));
        item->setPriority(proc->getPriority());

        const auto& stats = proc->getStats();
        item->setWaitTime(stats.totalWaitTime);
        item->setCpuTime(stats.totalCpuTime);
        item->setPageFaults(stats.pageFaults);

        // Burst actual
        if (proc->hasMoreBursts()) {
            QString burst = proc->getCurrentBurstType() == BurstType::CPU ?
                "CPU" : "E/S";
            burst += QString("(%1)").arg(proc->getCurrentBurstDuration());
            item->setCurrentBurst(burst);
        }

        m_processItems.append(item);
    }

    // 4. Notificar a QML que el modelo cambió
    emit processListChanged();

    // 5. Actualizar métricas globales
    auto metrics = m_simulator->getSimulatorMetrics();
    setAvgWaitTime(metrics.avgWaitTime);
    setAvgTurnaroundTime(metrics.avgTurnaroundTime);
    setCpuUtilization(metrics.cpuUtilization * 100.0); // Convertir a %
}

QString ProcessMonitorViewModel::processStateToString(ProcessState state) const {
    switch (state) {
        case ProcessState::NEW: return "Nuevo";
        case ProcessState::READY: return "Listo";
        case ProcessState::RUNNING: return "Ejecutando";
        case ProcessState::BLOCKED: return "Bloqueado (E/S)";
        case ProcessState::WAITING_MEMORY: return "Bloqueado (Memoria)";
        case ProcessState::TERMINATED: return "Terminado";
        default: return "Desconocido";
    }
}
```

### 5.2 En MemoryMonitorViewModel

```cpp
void MemoryMonitorViewModel::onClockTicked(uint64_t tick) {
    if (!m_memoryManager) return;

    // 1. Obtener estado de todos los frames
    auto frames = m_memoryManager->getFrameStatus();

    // 2. Actualizar modelo de frames para QML
    qDeleteAll(m_frameItems);
    m_frameItems.clear();

    for (const auto& frameInfo : frames) {
        auto* item = new FrameItemModel(this);

        item->setFrameId(frameInfo.frameId);
        item->setOccupied(frameInfo.isOccupied);

        if (frameInfo.isOccupied) {
            QString label = QString("P%1:%2")
                .arg(frameInfo.ownerPid)
                .arg(frameInfo.pageNumber);
            item->setLabel(label);
            item->setColor("#4CAF50"); // Verde
        } else {
            item->setLabel("Libre");
            item->setColor("#9E9E9E"); // Gris
        }

        m_frameItems.append(item);
    }

    emit frameListChanged();

    // 3. Actualizar estadísticas de memoria
    auto stats = m_memoryManager->getMemoryStats();
    setTotalPageFaults(stats.totalPageFaults);
    setTotalReplacements(stats.totalReplacements);
    setHitRatio(stats.hitRatio * 100.0); // Convertir a %
}
```

---

## 6. Prioridad de Implementación

Estas APIs son **imprescindibles** para tener una GUI funcional básica:

| API                        | Componente     | Justificación                     |
| -------------------------- | -------------- | --------------------------------- |
| `getAllProcesses()`        | Simulator      | Sin esto no hay lista de procesos |
| `getRunningProcess()`      | Simulator      | Necesario para mostrar CPU activa |
| `getBlockedProcesses()`    | Simulator      | Requerido para panel de bloqueos  |
| `peekReadyQueue()`         | IScheduler     | Mostrar cola Ready                |
| `getFrameStatus()`         | IMemoryManager | Visualización de memoria física   |
| `getPageTableForProcess()` | IMemoryManager | Tabla de páginas por proceso      |

Necesarias para cumplir especificaciones de entrega completas:

| API                     | Componente       | Justificación                         |
| ----------------------- | ---------------- | ------------------------------------- |
| `getSimulatorMetrics()` | Simulator        | Métricas requeridas en especificación |
| `getMemoryStats()`      | IMemoryManager   | Contadores de faults/replacements     |
| `getAlgorithmName()`    | Scheduler/Memory | Mostrar algoritmos activos            |
| `getMemoryWaitQueue()`  | Simulator        | Distinguir bloqueos E/S vs PageFault  |

Mejoras y features avanzados:

| API                     | Componente     | Justificación                |
| ----------------------- | -------------- | ---------------------------- |
| `getSchedulerMetrics()` | IScheduler     | Estadísticas detalladas      |
| `getReadyQueueSize()`   | IScheduler     | Útil para debugging          |
| `getTotalFrames()`      | IMemoryManager | Validaciones y configuración |

---

## 📝 Notas Importantes

### Thread Safety

- **TODOS** los getters deben proteger acceso con mutex (`std::lock_guard`).
- El `Simulator` ya tiene `m_simulationMutex`, usarlo en todos los métodos.
- Schedulers y Memory Managers deben agregar sus propios mutex si no los tienen.

### Performance

- Los getters retornan **copias** de datos, no referencias mutables.
- Para listas grandes, considerar usar `std::move` en retorno si es apropiado.
- Evitar cálculos costosos en getters; mantener métricas pre-calculadas.

### Actualización de Métricas

- `SimulatorMetrics`, `SchedulerMetrics` y `MemoryStats` deben **actualizarse en cada tick**.
- NO calcular bajo demanda en los getters (sería muy costoso).
- Mantener contadores como miembros privados y actualizarlos en `tick()` / `step()`.

### Compatibilidad

- Usar tipos C++ estándar en las estructuras (no Qt types como `QString`).
- La GUI hará la conversión a Qt types cuando sea necesario.
- Todas las estructuras en `waos::common` namespace.

---

## 🎯 Checklist de Implementación

Equipo Core:

- [ ] Crear `include/waos/common/DataStructures.h`
- [ ] Agregar getters a `Simulator.h`
- [ ] Implementar getters en `Simulator.cpp` con locks
- [ ] Agregar cálculo de métricas en `step()` / `tick()`

Equipo Scheduler:

- [ ] Agregar métodos virtuales a `IScheduler.h`
- [ ] Implementar en `FCFSScheduler`
- [ ] Implementar en `RRScheduler`
- [ ] Implementar en `PriorityScheduler`
- [ ] Implementar en `SJFScheduler`

Equipo Memory:

- [ ] Agregar métodos virtuales a `IMemoryManager.h`
- [ ] Implementar gestor concreto (FIFO/LRU/Optimal)
- [ ] Mantener contadores de hits/faults/replacements
- [ ] Implementar getters con locks

---

**Última actualización**: 24 de noviembre de 2025  
**Versión**: 1.0  
**Contacto GUI Team**: Issue #24 en GitHub
