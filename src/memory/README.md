# Módulo `memory`

Este directorio contiene las estructuras de datos y algoritmos de **gestión de memoria virtual** del simulador de sistema operativo. Implementa la simulación de paginación bajo demanda y algoritmos de reemplazo de páginas.

## Componentes Principales

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
Pendiente ...