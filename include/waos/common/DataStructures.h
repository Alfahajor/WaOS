/**
 * @file DataStructures.h
 * @brief Estructuras de datos compartidas entre GUI y Backend
 * @version 1.0
 * @date 24-11-2025
 *
 * Este archivo contiene las definiciones de estructuras de datos que son
 * utilizadas tanto por el backend (Simulator, Scheduler, Memory) como por
 * la GUI (ViewModels) para intercambiar información de manera consistente.
 *
 * IMPORTANTE: Usar solo tipos C++ estándar, NO tipos de Qt.
 */

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace waos::common {
/**
 * @struct MemoryWaitInfo
 * @brief Información sobre procesos esperando carga de página.
 * Útil para visualización en el Panel de Bloqueos (distinguir E/S de PageFault).
 */
struct MemoryWaitInfo {
  int pid;                // ID del proceso
  int pageNumber;         // Página que causó el fallo
  int ticksRemaining;     // Tiempo restante de penalización
};

/**
 * @brief Métricas globales del simulador calculadas en cada tick.
 *
 * IMPORTANTE: Estos valores deben actualizarse automáticamente en cada tick
 * del simulador, no calcularlos bajo demanda para evitar overhead.
 */
struct SimulatorMetrics {
  uint64_t currentTick = 0;        ///< Tick actual del reloj del simulador
  double avgWaitTime = 0.0;        ///< Tiempo promedio de espera (solo procesos terminados)
  double avgTurnaroundTime = 0.0;  ///< Tiempo promedio de retorno (solo procesos terminados)
  double cpuUtilization = 0.0;     ///< Porcentaje de ticks con CPU ocupada (0.0 - 1.0)
  int totalContextSwitches = 0;    ///< Cambios de contexto acumulados
  int totalPageFaults = 0;         ///< Page faults acumulados de todos los procesos
  int completedProcesses = 0;      ///< Procesos en estado TERMINATED
  int totalProcesses = 0;          ///< Total de procesos cargados en la simulación
};

/**
 * @brief Información de un frame de memoria física.
 *
 * Retornado por IMemoryManager::getFrameStatus()
 * Permite visualizar el estado de cada frame en el grid de memoria de la GUI.
 */
struct FrameInfo {
  int frameId;            ///< ID del frame (0 a N-1)
  bool isOccupied;        ///< true si contiene una página válida
  int ownerPid;           ///< PID del proceso dueño (-1 si libre)
  int pageNumber;         ///< Número de página virtual cargada (-1 si libre)
  uint64_t loadedAtTick;  ///< Tick cuando se cargó la página (0 si nunca)
};

/**
 * @brief Entrada de tabla de páginas de un proceso.
 *
 * Retornado por IMemoryManager::getPageTableForProcess()
 * Representa una entrada en la tabla de páginas de un proceso específico.
 */
struct PageTableEntryInfo {
  int pageNumber;   ///< Número de página virtual (0 a N-1)
  int frameNumber;  ///< Frame físico asignado (-1 si no presente en memoria)
  bool present;     ///< true si la página está cargada en memoria física
  bool referenced;  ///< Bit de referencia (usado por algoritmos LRU)
  bool modified;    ///< Bit de modificación / dirty bit
};

/**
 * @brief Estadísticas del gestor de memoria.
 *
 * Retornado por IMemoryManager::getMemoryStats()
 * Contiene métricas acumuladas del rendimiento del sistema de memoria.
 */
struct MemoryStats {
  int totalFrames;                      ///< Total de frames en memoria física
  int usedFrames;                       ///< Frames actualmente ocupados
  int totalPageFaults;                  ///< Page faults acumulados desde inicio
  int totalReplacements;                ///< Reemplazos de página realizados
  double hitRatio;                      ///< Ratio de hits (0.0 - 1.0)
  std::map<int, int> faultsPerProcess;  ///< Page faults por PID
};

/**
 * @brief Métricas del planificador.
 *
 * Retornado por IScheduler::getSchedulerMetrics()
 * Permite analizar el comportamiento del algoritmo de planificación.
 */
struct SchedulerMetrics {
  int totalSchedulingDecisions;       ///< Veces que se llamó getNextProcess()
  int totalPreemptions;               ///< Preempciones por quantum o prioridad
  std::map<int, int> selectionCount;  ///< Veces que cada PID fue seleccionado para ejecutar
};

}  // namespace waos::common
