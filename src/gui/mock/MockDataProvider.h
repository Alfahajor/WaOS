/**
 * @file MockDataProvider.h
 * @brief Proveedor de datos mock para desarrollo inicial de la GUI
 * @version 1.0
 * @date 24-11-2025
 *
 * Este componente genera datos estructurados realistas que simulan las
 * respuestas de las APIs del backend, permitiendo desarrollar y probar
 * la GUI antes de que las implementaciones reales estén disponibles.
 */

#pragma once

#include <memory>
#include <random>
#include <vector>

#include "waos/common/DataStructures.h"
#include "waos/core/Process.h"

namespace waos::gui::mock {

/**
 * @class MockDataProvider
 * @brief Generador de datos mock para la GUI
 *
 * Provee métodos estáticos para generar datos que simulan el estado
 * del simulador en diferentes momentos de la ejecución.
 */
class MockDataProvider {
 public:
  /**
   * @brief Genera una lista de procesos mock con estados variados
   * @param tick Tick actual de la simulación
   * @return Vector de procesos con estados realistas
   */
  static std::vector<std::unique_ptr<waos::core::Process>> generateProcessList(uint64_t tick);

  /**
   * @brief Genera métricas globales del simulador
   * @param tick Tick actual de la simulación
   * @return Struct SimulatorMetrics con valores calculados
   */
  static waos::common::SimulatorMetrics generateSimulatorMetrics(uint64_t tick);

  /**
   * @brief Genera estado de frames de memoria
   * @param totalFrames Cantidad total de frames
   * @param tick Tick actual de la simulación
   * @return Vector de FrameInfo representando memoria física
   */
  static std::vector<waos::common::FrameInfo> generateFrameStatus(int totalFrames, uint64_t tick);

  /**
   * @brief Genera tabla de páginas para un proceso
   * @param pid PID del proceso
   * @param pages Cantidad de páginas del proceso
   * @param tick Tick actual de la simulación
   * @return Vector de PageTableEntryInfo
   */
  static std::vector<waos::common::PageTableEntryInfo> generatePageTable(int pid, int pages, uint64_t tick);

  /**
   * @brief Genera estadísticas de memoria
   * @param tick Tick actual de la simulación
   * @return Struct MemoryStats con valores acumulados
   */
  static waos::common::MemoryStats generateMemoryStats(uint64_t tick);

  /**
   * @brief Genera métricas del planificador
   * @param tick Tick actual de la simulación
   * @return Struct SchedulerMetrics con estadísticas
   */
  static waos::common::SchedulerMetrics generateSchedulerMetrics(uint64_t tick);

 private:
  static std::mt19937& getRNG();
  static int randomInt(int min, int max);
  static double randomDouble(double min, double max);
};

}  // namespace waos::gui::mock
