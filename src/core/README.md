# :shell: Módulo `core`

Este directorio contiene las clases y estructuras de datos
fundamentales que forman el núcleo del simulador de sistema operativo.

## :hammer_and_pick: Componentes Principales

### `Process` (PCB)
La clase `Process` representa el **Process Control Block (PCB)**. Es
una estructura de datos pasiva que contiene toda la información de un
proceso.

-   **Responsabilidad:** Almacenar el estado, identificadores,
    parámetros de planificación, requisitos de memoria y estadísticas
    de rendimiento.
-   **No Responsabilidad:** No contiene lógica de ejecución,
    planificación o gestión de memoria. Es un contenedor de datos puro.

### `Clock`
La clase `Clock` es el corazón temporal de la simulación.

-   **Responsabilidad:** Mantener y avanzar el tiempo global de la
    simulación en unidades discretas (ticks).

> [!IMPORTANT]
> Una única instancia de `Clock` es gestionada por la clase
> `Simulator`. Los demás módulos nunca deben crear su propia
> instancia de `Clock`; en su lugar, recibirán el tiempo actual
> como un parámetro (`uint64_t`) de los métodos del `Simulator`.

## :skull: ¿Cómo Usar estas Clases?

Los módulos externos (como `scheduler` y `memory`) interactuarán con
estas clases a través de punteros o referencias proporcionados por el
`Simulator`.

### Ejemplo de Interacción (Scheduler)

Un algoritmo de planificación podría recibir un conjunto de procesos y
tomar una decisión basándose en sus datos.

```cpp
// Dentro de una clase FCFSScheduler, por ejemplo:
#include "waos/core/Process.h"

// El planificador mantiene una cola de punteros a procesos
std::queue<Process*> m_readyQueue;

void FCFSScheduler::addProcess(Process* process) {
  // El planificador no es dueño del proceso, solo lo referencia.
  m_readyQueue.push(process);
}

Process* FCFSScheduler::getNextProcess() {
  if (m_readyQueue.empty()) {
    return nullptr;
  }
  Process* next = m_readyQueue.front();
  m_readyQueue.pop();
  return next;
}
```

## :zap: Buenas Prácticas para Extender el Núcleo

-   **No añadas lógica al PCB:** Si necesitas una nueva funcionalidad
    (ej. calcular la prioridad dinámica), crea esa lógica en el módulo
    correspondiente (`scheduler`), no dentro de la clase `Process`.
    `Process` solo debe almacenar el resultado (ej.
    `m_currentPriority`).
-   **El tiempo es un parámetro:** Evita que los módulos dependan
    directamente de la clase `Clock`. Diseña sus métodos para que
    reciban el `currentTime` como un `uint64_t`. Esto mejora el
    desacoplamiento y la capacidad de prueba.
