# Módulo `memory`

Este directorio contiene las implementaciones de los diferentes algoritmos de reemplazo de páginas que utiliza el simulador WaOS. Todos los gestores de memoria implementan la interfaz común `IMemoryManager` para garantizar la interoperabilidad con el núcleo del simulador.

## :gear: Arquitectura

### Interfaz Base: `IMemoryManager`

Todos los algoritmos de reemplazo heredan de la clase abstracta `IMemoryManager`, que define el contrato básico:

```cpp
class IMemoryManager {
public:
    virtual bool isPageLoaded(int processId, int pageNumber) const = 0;
    virtual PageRequestResult requestPage(int processId, int pageNumber) = 0;
    virtual void allocateForProcess(int processId, int requiredPages) = 0;
    virtual void freeForProcess(int processId) = 0;
    virtual void completePageLoad(int processId, int pageNumber) = 0;
    
    // Métodos opcionales para Optimal
    virtual void registerFutureReferences(int processId, const std::vector<int>& referenceString);
    virtual void advanceInstructionPointer(int processId);
    
    // Observación para GUI
    virtual std::vector<FrameInfo> getFrameStatus() const = 0;
    virtual std::vector<PageTableEntryInfo> getPageTableForProcess(int processId) const = 0;
    virtual MemoryStats getMemoryStats() const = 0;
    virtual std::string getAlgorithmName() const = 0;
    virtual void reset() = 0;
};
```

**Características clave:**
- **No gestión de memoria:** Los gestores no tienen ownership de los punteros a `Process`.
- **Thread-safety:** Todas las implementaciones utilizan `std::mutex` para proteger sus estructuras internas.
- **Paginación por demanda:** Las páginas se cargan solo cuando son necesarias (lazy loading).

---

## :hammer_and_pick: Componentes Principales

### 1. `Frame` (Marco Físico)
Estructura que representa un **bloque de memoria física** (RAM) donde se carga una página.

-   **Datos que gestiona:** PID propietario, número de página lógica, estado de ocupación, timestamps para FIFO y LRU.
-   **No Responsabilidad:** No contiene lógica de reemplazo. Es un contenedor de datos puro.

```cpp
struct Frame {
    int pid = -1;               // Proceso propietario (-1 = libre)
    int pageNumber = -1;        // Página lógica cargada (-1 = vacío)
    bool occupied = false;      // ¿Está ocupado?
    uint64_t loadTime = 0;      // Timestamp de carga (para FIFO)
    uint64_t lastAccessTime = 0;// Timestamp de último acceso (para LRU)
    
    bool isFree() const { return !occupied; }
    void reset();               // Libera el frame
};
```

### 2. `PageTableEntry` (Entrada de Tabla de Páginas)
Mapeo entre una página lógica y su frame físico correspondiente.

```cpp
struct PageTableEntry {
    int frameNumber = -1;       // Frame físico asignado (-1 = no presente)
    bool present = false;       // ¿Está en memoria RAM?
    uint64_t lastAccess = 0;    // Timestamp para LRU
    bool referenced = false;    // Bit de referencia
    bool modified = false;      // Dirty bit (¿se modificó?)
    
    bool isLoaded() const { return present; }
    void load(int frame, uint64_t currentTime);
    void evict();               // Marca como desalojada
};
```

### 3. `PageTable` (Tabla de Páginas)
Un mapa hash que contiene todas las entradas de un proceso.

```cpp
using PageTable = std::unordered_map<int, PageTableEntry>;
```

> [!NOTE]
> **Actualización**
> Cada proceso tiene su propia `PageTable` gestionada internamente por el Memory Manager.
> El simulador nunca accede directamente a estas tablas.

---

## Algoritmos Implementados

### 1. **FIFO Memory Manager** (First-In, First-Out)

**Archivo:** `FIFOMemoryManager.cpp` / `FIFOMemoryManager.h`

**Descripción:**  
Reemplaza la página que **lleva más tiempo en memoria física**, sin considerar su frecuencia de uso.

**Estructura de datos:** `std::queue<std::pair<int, int>>` (cola de pares <PID, pageNumber>)

**Ventajas:**
- Simple de implementar
- Complejidad O(1) para selección de víctima
- Predecible

**Desventajas:**
- Sufre de la "Anomalía de Belady" (más frames puede causar más page faults)
- No considera localidad temporal

---

### 2. **LRU Memory Manager** (Least Recently Used)

**Archivo:** `LRUMemoryManager.cpp` / `LRUMemoryManager.h`

**Descripción:**  
Reemplaza la página que **no se ha accedido durante el mayor periodo de tiempo**.

**Estructura de datos:** No requiere estructuras adicionales (usa timestamps en `Frame` y `PageTableEntry`)

**Ventajas:**
- Mejor rendimiento que FIFO en la mayoría de casos
- Aprovecha localidad temporal

**Desventajas:**
- Mayor overhead (actualiza timestamps en cada acceso)
- Complejidad O(n) para selección de víctima

---

### 3. **Optimal Memory Manager** (Belady's Algorithm)

**Archivo:** `OptimalMemoryManager.cpp` / `OptimalMemoryManager.h`

**Descripción:**  
Reemplaza la página que **se usará más tarde en el futuro** (o nunca). Algoritmo teórico imposible de implementar en sistemas reales.

**Estructura de datos:** `std::unordered_map<int, ProcessFutureReferences>` (referencias futuras por proceso)

```cpp
struct ProcessFutureReferences {
    int processId;
    std::vector<int> futurePages;  // Secuencia completa de referencias
    size_t currentIndex;            // Posición actual
};
```

**Ventajas:**
- Rendimiento óptimo (menor cantidad de page faults posible)
- Benchmark teórico para comparar algoritmos prácticos

**Desventajas:**
- **Imposible en sistemas reales** (requiere conocimiento del futuro)
- Complejidad O(n*m) en selección de víctima

> [!IMPORTANT]
> **Requisitos especiales para Optimal:**
> - Debe invocarse `registerFutureReferences()` al asignar un proceso
> - Debe invocarse `advanceInstructionPointer()` tras cada tick de CPU
> - El simulador es responsable de estas llamadas adicionales

---

## Flujo de Datos y Restricciones Técnicas

1.  **Propiedad de los Datos:** La clase `Simulator` es la **dueña** de la instancia de `IMemoryManager` (`std::unique_ptr`). Nunca elimines manualmente el memory manager.

2.  **Inmutabilidad Relativa:**
    -   El `Simulator` invoca todos los métodos del Memory Manager.
    -   Los schedulers y GUI **NO** deben interactuar directamente con la memoria.
    -   Solo el Simulator modifica el estado de memoria.

3.  **Thread-Safety:** Todas las implementaciones protegen sus estructuras críticas con `std::mutex`. Es seguro llamar métodos desde threads diferentes.

4.  **Retornos por Valor:** Los métodos de observación (`getFrameStatus`, `getMemoryStats`) retornan **copias** de las estructuras, nunca referencias. Esto evita race conditions con la GUI.

> [!WARNING]
> **Para Optimal ÚNICAMENTE:**
> - El simulador DEBE invocar `registerFutureReferences()` después de `allocateForProcess()`
> - El simulador DEBE invocar `advanceInstructionPointer()` tras cada tick de CPU
> - Olvidar estas llamadas causará decisiones de reemplazo incorrectas

---

## Testing

### Archivo de Pruebas: `test_memory_main.cpp`

Programa de integración que verifica la compilación y funcionalidad básica de todos los gestores de memoria.

### Enum `PageRequestResult`

El método `requestPage()` retorna uno de tres valores:

```cpp
enum class PageRequestResult {
    HIT,          // Página ya estaba cargada (no hubo fallo)
    PAGE_FAULT,   // Fallo de página, cargada en frame libre
    REPLACEMENT   // Fallo de página con reemplazo de víctima
};
```

El simulador debe manejar estos tres casos:
- `HIT`: Continuar ejecución sin penalty
- `PAGE_FAULT` o `REPLACEMENT`: Bloquear proceso y aplicar delay de I/O

---
