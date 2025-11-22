# Módulo `core`

Este directorio contiene las clases y estructuras de datos
fundamentales que forman el núcleo del simulador.
Es la base sobre la cual se construyen los módulos de
planificación, memoria e interfaz gráfica.

## :hammer_and_pick: Componentes Principales

### 1. `Process` (PCB)
La clase `Process` representa el **Process Control Block (PCB)**. Es
una estructura de datos pasiva que contiene toda la información de un
proceso.

-   **Datos que gestiona:** PID, estado actual, tiempo de llegada,
    estadísticas y la cola de ráfagas de ejecución.
-   **No Responsabilidad:** No contiene lógica de ejecución,
    planificación o gestión de memoria. Es un contenedor de datos puro.

> [!NOTE]
> **Actualización**
> Ahora las ráfagas no son simples enteros. Se utiliza una cola de
> estructuras `Burst` que distinguen entre procesamiento y espera.

#### Estructuras Auxiliares (`Burst` y `BurstType`)
Para cumplir con los requisitos de simulación de E/S, definimos:

```cpp
enum class BurstType { CPU, IO };

struct Burst {
    BurstType type; // ¿Es cálculo o espera de E/S?
    int duration;   // Duración en ticks
};
```

### 2. `Parser`
Una utilidad estática encargada de la lectura y validación de los
archivos de entrada `.txt`.

-   **Responsabilidad:** Leer el archivo línea por línea, validar el
    formato y convertir el texto en estructuras `ProcessInfo` (DTOs)
    listas para ser consumidas por el `Simulator`.
-   **Formato de Entrada Soportado:**
    ```text
    # Formato: PID  Llegada  Ráfagas(Tipo(duración)...)  Prioridad  Páginas
    P1 0 CPU(4),E/S(3),CPU(5) 1 4
    ```
    *Nota: El parser distingue automáticamente entre `CPU(...)` y
    `E/S(...)`.*

### 3. `Clock`
El corazón temporal de la simulación. Mantiene el tiempo global en
"ticks" discretos.

> [!IMPORTANT]
> Una única instancia de `Clock` es gestionada por la clase
> `Simulator`. Los módulos externos reciben el tiempo actual
> (`uint64_t`) como parámetro, nunca deben instanciar su propio reloj.

---

## :handshake: Guía de Integración

A continuación se detalla cómo cada módulo debe interactuar con las
estructuras del núcleo.

### :calendar: Para el módulo `scheduler` (Planificador)

El módulo recibirá punteros a objetos `Process`.

1.  **Enfoque en CPU:** El algoritmo solo debe preocuparse por
    procesos que están en estado `READY` y cuya ráfaga actual sea de
    tipo `BurstType::CPU`.
2.  **Consulta de Duración (SJF/RR):** Para algoritmos como SJF, usa
    `process->getCurrentBurstDuration()`.
3.  **Gestión de E/S:** Si la ráfaga actual de un proceso es
    `BurstType::IO`, **NO** es responsabilidad del planificador. El
    `Simulator` se encargará de moverlo a la cola de bloqueados
    automáticamente.

**Ejemplo de uso:**
```cpp
void SJFScheduler::addProcess(Process* p) {
    // Verificación defensiva (opcional, el simulador ya debería filtrar esto)
    if (p->getCurrentBurstType() == BurstType::CPU) {
        readyQueue.push(p);
    }
}

// Para ordenar la cola por duración:
int duration = p->getCurrentBurstDuration();
```

### :floppy_disk: Para el módulo  `memory` (Gestor de Memoria)

El `Process` es la unidad de carga.

1.  **Requisitos:** Usar `process->getRequiredPages()` para saber
    cuántos marcos necesita el proceso antes de ser admitido.
2.  **Identidad:** El `PID` (`process->getPid()`) es la clave para
    asociar páginas en la Tabla de Páginas Invertida o estructura
    similar.

### :desktop_computer: Para el módulo `gui` (Interfaz Gráfica)

La UI debe ser un observador pasivo.

1.  **Visualización de Estado:** Usar `process->getState()` para
    colorear las filas de la tabla de procesos (ej. Verde para
    RUNNING, Rojo para BLOCKED).
2.  **Estadísticas:** Accede a `process->getStats()` para mostrar
    métricas en tiempo real (tiempo de espera, CPU, etc.).
3.  **Progreso:** Puedes usar `process->getCurrentBurstDuration()`
    para animar barras de progreso.

---

## :zap: Flujo de Datos y Restricciones Técnicas

1.  **Propiedad de los Datos:** La clase `Simulator` (a implementar)
    es la **dueña** (`std::unique_ptr`) de todos los procesos. Los
    demás módulos solo reciben punteros crudos (`Process*`)
    temporales. **Nunca elimines (`delete`) un proceso en el
    scheduler o memory manager.**

2.  **Inmutabilidad Relativa:**
    -   El `Parser` crea la información inicial.
    -   El `Simulator` modifica el estado (`setState`) y avanza las
        ráfagas.
    -   El `Scheduler` y `MemoryManager` **NO** deben modificar el
        estado interno del proceso (ej. no cambiar manualmente
        `m_state`), solo leer información para tomar decisiones.

3.  **Manejo de Errores del Parser:** El parser está diseñado para
    ser robusto. Si encuentra una línea mal formada, imprimirá un
    *warning* en `stderr` y la saltará, permitiendo que la
    simulación continúe con los procesos válidos restantes.
