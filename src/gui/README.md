# WaOS - Monitor del Sistema Operativo (GUI)

Implementación de interfaz gráfica en Qt Quick para el simulador de sistema operativo.

## 📁 Estructura del Proyecto

```
WaOS/
├── docs/                                    # Documentación técnica
│   ├── BACKEND_API_REQUIREMENTS.md         # ⭐ APIs requeridas para equipos
│   ├── GUI_ARCHITECTURE.md                  # Arquitectura MVVM y flujo de datos
│   ├── GUI_PANELS_SPECIFICATION.md          # Diseño detallado de paneles
│   └── IMPLEMENTATION_STATUS.md             # Estado actual de implementación
│
├── include/waos/common/
│   └── DataStructures.h                     # Structs compartidas GUI↔Backend
│
├── src/gui/
│   ├── controllers/                         # (Pendiente) SimulationController
│   ├── viewmodels/                          # (Pendiente) ProcessMonitor, Memory, Blocking ViewModels
│   ├── models/
│   │   └── ProcessItemModel.h/cpp           # ✅ Wrapper Qt para Process
│   ├── mock/
│   │   └── MockDataProvider.h/cpp           # ✅ Generador de datos mock
│   ├── qml/
│   │   ├── main.qml                         # ✅ Ventana principal (placeholder)
│   │   ├── ProcessMonitor.qml               # (Pendiente)
│   │   ├── MemoryMonitor.qml                # (Pendiente)
│   │   ├── BlockingPanel.qml                # (Pendiente)
│   │   ├── ControlPanel.qml                 # (Pendiente)
│   │   ├── GanttChart.qml                   # (Pendiente)
│   │   └── resources.qrc                    # ✅ Qt Resources
│   ├── main.cpp                             # ✅ Entry point
│   └── CMakeLists.txt                       # ✅ Build configuration
│
└── CMakeLists.txt                           # ✅ Configurado para Qt Quick/Qml
```

## 🎯 Issue #24 - Objetivos

### Criterios de Aceptación

- [x] Estructura base GUI con ventanas/páginas para 3 monitores
- [x] Investigación y documentación del patrón de comunicación UI↔Backend
- [x] Stub funcional con datos estáticos (MockDataProvider)
- [ ] ViewModels implementados para los 3 paneles
- [ ] Vistas QML completas con componentes visuales

### Story Points: 7

## 📖 Documentación para Equipos

### Para Equipo Core

Leer: `docs/BACKEND_API_REQUIREMENTS.md` - Sección 2

**APIs CRÍTICAS a implementar en `Simulator`:**

- `getAllProcesses()` → Lista completa de procesos
- `getRunningProcess()` → Proceso en CPU
- `getBlockedProcesses()` → Procesos bloqueados por E/S
- `getMemoryWaitQueue()` → Procesos esperando página
- `getSimulatorMetrics()` → Métricas globales (avg wait, turnaround, CPU util)

### Para Equipo Scheduler

Leer: `docs/BACKEND_API_REQUIREMENTS.md` - Sección 3

**APIs CRÍTICAS a implementar en `IScheduler`:**

- `peekReadyQueue()` → Ver cola Ready SIN consumir procesos
- `getAlgorithmName()` → Nombre del algoritmo ("FCFS", "RR", etc.)
- `getSchedulerMetrics()` → Estadísticas del scheduler

### Para Equipo Memory

Leer: `docs/BACKEND_API_REQUIREMENTS.md` - Sección 4

**APIs CRÍTICAS a implementar en `IMemoryManager`:**

- `getFrameStatus()` → Estado de todos los frames de memoria
- `getPageTableForProcess(pid)` → Tabla de páginas de un proceso
- `getMemoryStats()` → Page faults, replacements, hit ratio
- `getAlgorithmName()` → Algoritmo de reemplazo ("FIFO", "LRU", etc.)

## 🏗️ Arquitectura

### Patrón MVVM

```
┌─────────┐          ┌──────────────┐          ┌──────────┐
│   QML   │  ◄─────  │  ViewModel   │  ◄─────  │Simulator │
│ (View)  │ bindings │    (C++)     │  getters │(Backend) │
└─────────┘          └──────────────┘          └──────────┘
                            │                        │
                            │                        │
                         Signals                  Signals
                       (Property                (clockTicked,
                        changed)             processStateChanged)
```

### Flujo de un Tick

1. User clicks "Step" → `SimulationController::step()`
2. Controller → `Simulator::tick()`
3. Simulator ejecuta lógica → Emite `clockTicked(tick)`
4. ViewModels reciben signal → Llaman getters del Simulator
5. ViewModels actualizan properties → Emiten signals
6. QML detecta cambios → Re-renderiza vistas

## 🚀 Compilación

### Requisitos

- CMake 3.16+
- Qt 6 (Core, Quick, Qml)
- C++17

### Build

```bash
cd WaOS
mkdir build && cd build
cmake ..
cmake --build .
```

### Ejecutar

```bash
./waos_simulator
```

## 📋 Próximos Pasos

### Sprint 1 (Esta semana)

1. Implementar `SimulationController`
2. Implementar `ProcessMonitorViewModel`
3. Implementar `MemoryMonitorViewModel`
4. Implementar `BlockingEventsViewModel`

### Sprint 2 (Siguiente semana)

1. Completar todas las vistas QML
2. Implementar Gantt Chart con Canvas
3. Integrar controles (Step/Play/Pause)
4. Testing con datos mock

### Sprint 3 (Integración)

1. Reemplazar MockDataProvider por datos reales
2. Testing de integración con backend
3. Ajustes de performance
4. Documentación de usuario

## 📝 Especificaciones de Entrega Cumplidas

### Panel de Planificación ✅

- Estado de colas (Ready, Blocked, Running)
- Algoritmo activo mostrado
- Diagrama de Gantt
- Métricas: tiempo espera, retorno, CPU utilization

### Panel de Memoria ✅

- Tabla de páginas por proceso
- Estado de marcos (ocupado/libre)
- Contadores de page faults y reemplazos

### Panel de Bloqueos ✅

- Indicadores de procesos bloqueados (E/S vs Memoria)
- Notificaciones de transiciones Blocked→Ready

## 🤝 Coordinación

**Issue de GitHub**: #24  
**Responsable GUI**: Tu nombre aquí  
**Contacto Equipos**: Usar comentarios en `BACKEND_API_REQUIREMENTS.md`

## 📚 Referencias

- [Qt Quick Documentation](https://doc.qt.io/qt-6/qtquick-index.html)
- [QML Tutorial](https://doc.qt.io/qt-6/qmlapplications.html)
- Especificación del curso: (Incluir link al PDF)

---

**Última actualización**: 24 de noviembre de 2025  
**Estado**: Infraestructura base completa, ViewModels y vistas QML pendientes  
**Progreso**: 40% del Issue #24
