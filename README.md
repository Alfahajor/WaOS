# <samp>WaOS Simulator</samp>

**Simulador Integrado de Planificación de Procesos y Gestión de 
Memoria Virtual**

Este proyecto es un simulador educativo desarrollado en C++ y Qt6 que
modela el comportamiento de un sistema operativo simplificado. Integra
dos módulos clave: un planificador de CPU y un gestor de memoria
virtual.

## Módulos Principales

-   **Core:** La biblioteca central que orquesta la simulación,
    gestiona el tiempo, los procesos y la comunicación entre módulos.
-   **Scheduler:** Implementa diversos algoritmos de planificación de
    CPU (FCFS, SJF, Round Robin).
-   **Memory Manager:** Implementa algoritmos de reemplazo de páginas
    (FIFO, LRU, Óptimo).
-   **UI:** La interfaz gráfica de usuario construida con Qt6 para
    visualizar la simulación en tiempo real.

### Estructura de directorios y archivos

La estructura se divide en los módulos descritos anteriormente

```sh
WaOS/
├── .github/
│   ├── ISSUE_TEMPLATE/
│   ├── PULL_REQUEST_TEMPLATE.md
│   └── workflows/
├── .gitignore
├── CMakeLists.txt
├── README.md
├── core/
├── docs/
├── include/
│   └── waos/
│       ├── core/
│       ├── memory/
│       └── scheduler/
├── src/
│   ├── core/
│   ├── memory/
│   ├── scheduler/
│   ├── ui/
│   └── main.cpp
└── tests/
    └── core/
```

## Requisitos de Compilación

-   CMake 3.16+
-   Compilador C++17 (GCC, Clang, MSVC)
-   Qt 6.2+

## Instrucciones de Compilación

1.  **Clonar el repositorio:**
    ```bash
    git clone <URL_DEL_REPOSITORIO>
    cd WaOS/
    ```

2.  **Configurar con CMake:**
    ```bash
    cmake -B build
    ```

3.  **Compilar el proyecto:**
    ```bash
    cmake --build build
    ```

4.  **Ejecutar el simulador:**
    ```bash
    ./build/simulator
    ```
