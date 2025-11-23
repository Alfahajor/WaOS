# Módulo `scheduler`

Este directorio contiene las implementaciones de los diferentes algoritmos de planificación de procesos (CPU schedulers) que utiliza el simulador WaOS. Todos los planificadores implementan la interfaz común `IScheduler` para garantizar la interoperabilidad con el núcleo del simulador.

## :gear: Arquitectura

### Interfaz Base: `IScheduler`

Todos los algoritmos de planificación heredan de la clase abstracta `IScheduler`, que define el contrato básico:

```cpp
class IScheduler {
public:
    virtual void addProcess(waos::core::Process* p) = 0;
    virtual waos::core::Process* getNextProcess() = 0;
    virtual bool hasReadyProcesses() const = 0;
};
```

**Características clave:**
- **No gestión de memoria:** Los planificadores no tienen ownership de los punteros a `Process`.
- **Thread-safety:** Las implementaciones que lo requieren utilizan `std::mutex` para proteger sus estructuras internas.
- **Consumo de procesos:** `getNextProcess()` devuelve Y elimina el proceso de la cola interna.

---

## Algoritmos Implementados

### 1. **FCFS Scheduler** (First-Come, First-Served)

**Archivo:** `FCFSScheduler.cpp` / `FCFSScheduler.h`

**Descripción:**  
El algoritmo más simple. Los procesos se ejecutan en el orden exacto en que llegan (FIFO).

**Estructura de datos:** `std::queue<Process*>`

---

### 2. **SJF Scheduler** (Shortest Job First) [STUB]

**Archivo:** `SJFScheduler.cpp` / `SJFScheduler.h`

**Descripción:**  
Selecciona el proceso con la **ráfaga de CPU más corta** para ejecutar a continuación.

**Estado actual:** 🚧 **Implementación parcial (stub)**  
- Actualmente solo almacena procesos en un vector sin ordenar
- Selección: retorna el primer elemento (comportamiento FCFS temporal)

**Estructura de datos:** `std::vector<Process*>` (protegido con `std::mutex`)

---

### 3. **Round Robin Scheduler** [STUB]

**Archivo:** `RRScheduler.cpp` / `RRScheduler.h`

**Descripción:**  
Asigna a cada proceso un quantum de tiempo fijo. Si el proceso no termina, se re-encola al final.

**Estado actual:** 🚧 **Implementación parcial (stub)**  
- ⚠️ **Limitación crítica:** Los procesos NO se re-encolan automáticamente
- Actualmente funciona como FCFS con quantum ignorado
- El quantum se configura en el constructor (default: 100ms)

**Estructura de datos:** `std::queue<Process*>` (protegido con `std::mutex`)

**Características configurables:**
```cpp
// Constructor con quantum personalizado
RRScheduler scheduler(std::chrono::milliseconds(50));
```
---

### 4. **Priority Scheduler** [STUB]

**Archivo:** `PriorityScheduler.cpp` / `PriorityScheduler.h`

**Descripción:**  
Selecciona procesos basándose en su nivel de prioridad. Los procesos con mayor prioridad se ejecutan primero.

**Estado actual:** 🚧 **Implementación parcial (stub)**  
- ⚠️ **Todos los procesos reciben prioridad por defecto = 0**
- La clase `Process` aún no expone un método `getPriority()`

**Estructura de datos:** `std::map<int, std::deque<Process*>>` (protegido con `std::mutex`)  
- Clave: nivel de prioridad (menor número = mayor prioridad)
- Valor: cola FIFO de procesos con esa prioridad

---

## Testing

### Archivo de Pruebas: `test_scheduler_main.cpp`

Programa de integración que verifica la compilación y funcionalidad básica de todos los planificadores.

### Filtrado de Procesos con E/S

> [!IMPORTANT]
> Los schedulers **solo deben gestionar procesos con ráfagas de CPU**.  
> El simulador es responsable de:
> - Detectar cuando `getCurrentBurstType() == BurstType::IO`
> - Mover el proceso a la cola de bloqueados
> - Re-encolar en el scheduler cuando la E/S termine

---

**Estado general del módulo:** 🟡 Stubs funcionales, implementación completa pendiente
