# 🧠 Módulo `memory`

Este directorio contiene las estructuras de datos y algoritmos de **gestión de memoria virtual** del simulador de sistema operativo. Implementa la simulación de paginación bajo demanda y algoritmos de reemplazo de páginas.

## 🏗️ Componentes Principales

### Estructuras de Datos Base

#### `Frame`
La estructura `Frame` representa un **marco de memoria física** (un bloque de RAM).

-   **Responsabilidad:** Almacenar información sobre qué proceso y qué página lógica ocupa el marco físico.
-   **Campos clave:**
    -   `pid`: ID del proceso dueño (-1 si está libre)
    -   `pageNumber`: Número de página lógica mapeada
    -   `occupied`: Bandera de ocupación
    -   `loadTime`: Timestamp de carga (usado por FIFO)
    -   `lastAccessTime`: Timestamp de último acceso (usado por LRU)
-   **No Responsabilidad:** No contiene lógica de asignación o reemplazo. Es un contenedor de datos puro.

#### `PageTableEntry`
La estructura `PageTableEntry` representa una **entrada en la tabla de páginas** de un proceso.

-   **Responsabilidad:** Mapear una página lógica a un marco físico y mantener bits de control.
-   **Campos clave:**
    -   `frameNumber`: Número de marco físico donde está la página (-1 si no está cargada)
    -   `present`: Bit de presencia (true = en memoria, false = en disco)
    -   `lastAccess`: Timestamp de último acceso
    -   `referenced`: Bit de referencia
    -   `modified`: Bit de modificación (dirty bit)
-   **Métodos auxiliares:**
    -   `isLoaded()`: Verifica si la página está en memoria
    -   `load(frame, time)`: Marca la página como cargada
    -   `evict()`: Marca la página como desalojada

#### `PageTable`
Alias de tipo que define la **tabla de páginas** como un mapa hash.

-   **Definición:** `using PageTable = std::unordered_map<int, PageTableEntry>`
-   **Responsabilidad:** Mapear números de página lógica (int) a sus entradas correspondientes (PageTableEntry).
-   **Ventaja:** Cada proceso tiene su propia tabla de páginas, permitiendo espacios de direcciones independientes.

### Interfaz Abstracta

#### `IMemoryManager`
La clase abstracta `IMemoryManager` define el **contrato** que todos los algoritmos de gestión de memoria deben implementar.

-   **Responsabilidad:** Definir la API común para todos los gestores de memoria.
-   **Métodos principales:**
    -   `handlePageRequest(Process* p, int pageNumber)`: Maneja una solicitud de acceso a página
    -   `allocateProcess(Process* p)`: Crea estructuras para un nuevo proceso
    -   `deallocateProcess(int pid)`: Libera estructuras de un proceso terminado
-   **Métodos de estadísticas:**
    -   `getPageFaults()`: Contador total de fallos de página
    -   `getPageReplacements()`: Contador total de reemplazos
    -   `getFreeFrames()`: Marcos libres disponibles
-   **Patrón de diseño:** Strategy pattern - permite intercambiar algoritmos sin cambiar el código del `Simulator`.

### Implementaciones de Algoritmos

#### `FIFOManager`
Implementación del algoritmo **First-In, First-Out** (FIFO).

-   **Principio:** Reemplaza la página que lleva más tiempo en memoria (la más antigua).
-   **Estructura específica:** Usa una `std::queue<int>` para rastrear el orden de llegada de los marcos.
-   **Ventaja:** Simplicidad - O(1) para selección de víctima.
-   **Desventaja:** Puede sufrir la anomalía de Belady (más memoria = más page faults).
-   **Estado:** ✅ **Implementación completa y funcional**

#### `LRUManager`
Implementación del algoritmo **Least Recently Used** (LRU).

-   **Principio:** Reemplaza la página que no se ha usado durante más tiempo.
-   **Estado:** ⚠️ **Stub compilable - Pendiente de implementación**
-   **Nota:** Usa `lastAccessTime` del `PageTableEntry` para rastrear accesos.

#### `OptimalManager`
Implementación del algoritmo **Óptimo** (teórico).

-   **Principio:** Reemplaza la página que no se usará durante más tiempo en el futuro.
-   **Estado:** ⚠️ **Stub compilable - Pendiente de implementación**
-   **Nota:** Requiere conocimiento previo de la secuencia de referencias futuras.

## 🎯 ¿Cómo Usar estas Clases?

El módulo `memory` será utilizado por la clase `Simulator` a través de la interfaz `IMemoryManager`, permitiendo seleccionar dinámicamente el algoritmo de reemplazo.

### Ejemplo de Uso (desde Simulator)

```cpp
#include "waos/memory/FIFOManager.h"
#include "waos/core/Process.h"

// El Simulator crea un gestor de memoria con 10 marcos físicos
IMemoryManager* memoryManager = new FIFOManager(10);

// Cuando llega un proceso nuevo
Process* p1 = new Process(1, 0, bursts, 5); // 5 páginas requeridas
memoryManager->allocateProcess(p1);

// Cuando el proceso solicita acceder a la página 3
bool success = memoryManager->handlePageRequest(p1, 3);
if (!success) {
  // Hubo page fault (ya manejado internamente)
  std::cout << "Page fault en página 3 del proceso 1\n";
}

// Cuando el proceso termina
memoryManager->deallocateProcess(1);

// Obtener estadísticas
uint64_t faults = memoryManager->getPageFaults();
uint64_t replacements = memoryManager->getPageReplacements();
```

### Ejemplo de Cambio de Algoritmo (Polimorfismo)

```cpp
// Fácil intercambio gracias a la interfaz abstracta
IMemoryManager* mgr;

if (userChoice == "FIFO") {
  mgr = new FIFOManager(totalFrames);
} else if (userChoice == "LRU") {
  mgr = new LRUManager(totalFrames);
} else {
  mgr = new OptimalManager(totalFrames);
}

// El resto del código funciona igual sin cambios
mgr->handlePageRequest(process, pageNum);
```

## 🔍 Arquitectura Interna

### Separación de Responsabilidades

```
┌─────────────────────────────────────────────────────────┐
│                      Simulator                           │
│  (orquestador - conoce solo IMemoryManager*)            │
└───────────────────────┬─────────────────────────────────┘
                        │ usa
                        ▼
┌─────────────────────────────────────────────────────────┐
│                  IMemoryManager                          │
│         (interfaz abstracta - contrato)                  │
└───┬─────────────────┬─────────────────┬─────────────────┘
    │                 │                 │
    │ implementa      │ implementa      │ implementa
    ▼                 ▼                 ▼
┌──────────┐    ┌──────────┐    ┌──────────────┐
│   FIFO   │    │   LRU    │    │   Optimal    │
│ Manager  │    │ Manager  │    │   Manager    │
└────┬─────┘    └────┬─────┘    └──────┬───────┘
     │               │                  │
     └───────────────┴──────────────────┘
                     │ usa
                     ▼
     ┌────────────────────────────────────┐
     │  Frame, PageTableEntry, PageTable  │
     │     (estructuras de datos base)     │
     └────────────────────────────────────┘
```

### Memoria Física vs Tablas de Páginas

**Concepto clave:**
-   **Memoria física** (`std::vector<Frame>`): **Una sola instancia compartida** por todos los procesos. Representa la RAM limitada del sistema.
-   **Tablas de páginas** (`std::unordered_map<int, PageTable>`): **Una por proceso**. Cada proceso tiene su propia tabla que mapea sus páginas lógicas a marcos físicos.

**Analogía del hotel:**
-   **Frames (memoria física)** = Habitaciones del hotel (limitadas, compartidas)
-   **PageTable (tabla de páginas)** = Registro de la recepción (uno por huésped)
-   **Page fault** = Cliente llega pero no hay habitación disponible → hay que desalojar a otro huésped (reemplazo)

## ⚡ Buenas Prácticas para Extender el Módulo

-   **Mantén las estructuras base simples:** `Frame` y `PageTableEntry` son contenedores de datos puros. No agregues lógica compleja ahí.
-   **Thread-safety:** Los managers usan `std::mutex` para operaciones seguras en entornos concurrentes. Respeta los locks existentes.
-   **El tiempo es un parámetro:** Los managers no conocen la clase `Clock` directamente. Usan `currentTime` como `uint64_t` pasado por el `Simulator`.
-   **Nuevos algoritmos:** Para agregar un nuevo algoritmo de reemplazo:
    1. Crea una clase que herede de `IMemoryManager`
    2. Implementa todos los métodos virtuales puros
    3. Añade la estructura de datos específica del algoritmo (ej: cola para FIFO, lista para LRU)
    4. Implementa `selectVictimFrame()` con tu lógica de selección

## 🧪 Testing

El módulo incluye dos archivos de test:

-   **`test_memory_main.cpp`**: Test completo de FIFO + stubs de LRU y Optimal
-   **`test_memory_core.cpp`**: Test básico solo de estructuras base (Frame, PageTableEntry, PageTable) sin algoritmos

Para compilar y ejecutar:
```bash
# Test completo (requiere GCC 11+ o compilador moderno)
.\compile_memory_test.ps1

# Test solo de estructuras base (compatible con GCC 6.3+)
.\compile_memory_core_test.ps1
```

## 📊 Estado del Módulo

| Componente | Estado | Descripción |
|------------|--------|-------------|
| `Frame` | ✅ Completo | Estructura de marco físico |
| `PageTableEntry` | ✅ Completo | Entrada de tabla de páginas |
| `PageTable` | ✅ Completo | Type alias para tabla |
| `IMemoryManager` | ✅ Completo | Interfaz abstracta |
| `FIFOManager` | ✅ Completo | Implementación FIFO funcional |
| `LRUManager` | ⏳ Pendiente | Stub compilable |
| `OptimalManager` | ⏳ Pendiente | Stub compilable |

## 🔗 Integración con otros módulos

-   **`core`**: Usa `Process*` para obtener PID y páginas requeridas
-   **`scheduler`**: El scheduler NO conoce directamente el módulo memory. El `Simulator` coordina ambos.
-   **`Simulator` (futuro)**: Será el orquestador que llame a `handlePageRequest()` cuando un proceso en ejecución necesite acceder a memoria.

---

> [!IMPORTANT]
> Al igual que con el módulo `core`, el `Simulator` es el único dueño de la instancia de `IMemoryManager`. Los demás módulos solo reciben referencias o interactúan a través de los métodos públicos de la interfaz.
