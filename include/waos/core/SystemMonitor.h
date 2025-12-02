#pragma once

#include <mutex>
#include <condition_variable>
#include "waos/core/Process.h"

namespace waos::core {

  /**
   * @class SystemMonitor
   * @brief Monitor de sincronización para coordinar la ejecución concurrente.
   * Centraliza el control de "quién puede usar la CPU".
   */
  class SystemMonitor {
  public:
    SystemMonitor() = default;
    ~SystemMonitor() = default;

    /**
     * @brief El Kernel autoriza a un proceso a ejecutar un ciclo de CPU.
     * @param p Puntero al proceso.
     */
    void dispatch(Process* p) {
      if (!p) return;
      // Despierta al hilo del proceso específico
      p->signalRun();
    }

    /**
     * @brief El Kernel espera a que el proceso termine su ráfaga actual (Tick).
     * Actúa como barrera de sincronización para garantizar determinismo paso a paso.
     * @param p Puntero al proceso.
     */
    void waitForBurstCompletion(Process* p) {
      if (!p) return;
      // Espera señal del hilo
      p->waitForTickCompletion();
    }
  };

}
