# GUI Architecture Documentation

**Arquitectura MVVM para Monitor del Sistema Operativo WaOS**

Este documento detalla la arquitectura de la GUI Qt Quick, el patrón de comunicación entre componentes, y el flujo de datos desde el backend hasta la visualización.

---

## 📋 Índice

1. [Arquitectura General](#arquitectura-general)
2. [Patrón MVVM Adoptado](#patrón-mvvm-adoptado)
3. [Flujo de Datos](#flujo-de-datos)
4. [Componentes Principales](#componentes-principales)
5. [Threading y Sincronización](#threading-y-sincronización)
6. [Transición de Mock a Datos Reales](#transición-de-mock-a-datos-reales)

---

## 1. Arquitectura General

### 1.1 Diagrama de Componentes

```
┌─────────────────────────────────────────────────────────────┐
│                      QML Layer (Vista)                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ProcessMonitor│  │MemoryMonitor │  │BlockingPanel │      │
│  │    .qml      │  │    .qml      │  │    .qml      │      │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘      │
│         │                  │                  │              │
│         └──────────────────┴──────────────────┘              │
│                            │                                 │
│                   ┌────────▼────────┐                        │
│                   │  ControlPanel   │                        │
│                   │     .qml        │                        │
│                   └────────┬────────┘                        │
└────────────────────────────┼──────────────────────────────────┘
                             │ Properties & Signals
┌────────────────────────────▼──────────────────────────────────┐
│                   ViewModel Layer (C++)                       │
│  ┌────────────────┐  ┌────────────────┐  ┌────────────────┐ │
│  │ProcessMonitor  │  │MemoryMonitor   │  │BlockingEvents  │ │
│  │  ViewModel     │  │  ViewModel     │  │  ViewModel     │ │
│  └───────┬────────┘  └───────┬────────┘  └───────┬────────┘ │
│          │                    │                    │          │
│          └────────────────────┴────────────────────┘          │
│                               │                               │
│                    ┌──────────▼──────────┐                    │
│                    │ SimulationController│                    │
│                    │     (QObject)       │                    │
│                    └──────────┬──────────┘                    │
└───────────────────────────────┼────────────────────────────────┘
                                │ Method Calls
┌───────────────────────────────▼────────────────────────────────┐
│                   Backend Layer (C++)                          │
│                    ┌──────────┐                                │
│                    │Simulator │                                │
│                    │(QObject) │                                │
│                    └─┬──────┬─┘                                │
│                      │      │                                  │
│              ┌───────▼──┐ ┌─▼─────────┐                        │
│              │IScheduler│ │IMemoryMgr │                        │
│              └──────────┘ └───────────┘                        │
└────────────────────────────────────────────────────────────────┘
```

### 1.2 Stack Tecnológico

- **Frontend**: Qt Quick (QML + JavaScript)
- **Backend**: C++17
- **Build System**: CMake 3.16+
- **Framework**: Qt 6 (Core, Quick, Qml)
- **Patrón**: MVVM (Model-View-ViewModel)

---

## 2. Patrón MVVM Adoptado

### 2.1 Responsabilidades por Capa

#### View (QML)

- **Responsabilidad**: Presentación visual y captura de eventos de usuario
- **Tecnología**: Qt Quick (QML)
- **Comunicación**: Solo con ViewModel mediante:
  - Properties binding (lectura reactiva)
  - Signal handlers (eventos)
  - Invocable methods (acciones)

**Ejemplo**:

```qml
// ProcessMonitor.qml
ListView {
    model: processViewModel.processList  // Property binding

    delegate: Rectangle {
        color: {
            switch(modelData.state) {
                case "Ejecutando": return "#4CAF50"  // Verde
                case "Listo": return "#FFC107"       // Amarillo
                case "Bloqueado (E/S)": return "#F44336"  // Rojo
                case "Bloqueado (Memoria)": return "#FF9800" // Naranja
                case "Terminado": return "#9E9E9E"   // Gris
            }
        }
    }
}

Button {
    text: "Step"
    onClicked: simulationController.step()  // Invocable method
}
```

#### ViewModel (C++ QObject)

- **Responsabilidad**: Lógica de presentación y adaptación de datos
- **Tecnología**: C++ con macros Qt (`Q_OBJECT`, `Q_PROPERTY`, `Q_INVOKABLE`)
- **Comunicación**:
  - Con View: Expone properties y signals
  - Con Model: Llama métodos del Simulator/Scheduler/Memory

**Ejemplo**:

```cpp
// ProcessMonitorViewModel.h
class ProcessMonitorViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> processList READ processList NOTIFY processListChanged)
    Q_PROPERTY(double avgWaitTime READ avgWaitTime NOTIFY metricsChanged)

public:
    QList<QObject*> processList() const { return m_processItems; }
    double avgWaitTime() const { return m_avgWaitTime; }

signals:
    void processListChanged();
    void metricsChanged();

public slots:
    void onClockTicked(uint64_t tick);

private:
    Simulator* m_simulator;
    QList<QObject*> m_processItems;
    double m_avgWaitTime = 0.0;
};
```

#### Model (C++ Backend)

- **Responsabilidad**: Lógica de negocio del simulador
- **Tecnología**: C++17 puro (con Qt signals para notificaciones)
- **Comunicación**: Emite signals cuando cambia estado

**Ejemplo**:

```cpp
// Simulator.h (ya existente)
class Simulator : public QObject {
    Q_OBJECT

signals:
    void clockTicked(uint64_t currentTime);
    void processStateChanged(int pid, ProcessState newState);
    void simulationFinished();
    void logMessage(QString message);
};
```

### 2.2 Separación de Concerns

| Capa          | Contiene                                      | NO Contiene                                 |
| ------------- | --------------------------------------------- | ------------------------------------------- |
| **QML**       | Layouts, animaciones, estilos, event handlers | Lógica de negocio, cálculos, acceso a datos |
| **ViewModel** | Formateo de datos, conversiones, estado de UI | Lógica de simulación, algoritmos            |
| **Model**     | Simulación, planificación, gestión de memoria | Detalles de presentación, Qt Quick types    |

---

## 3. Flujo de Datos

### 3.1 Inicialización de la Aplicación

```
1. main.cpp
   └─> QQmlApplicationEngine created
       └─> Register types: qmlRegisterType<SimulationController>("WaOS", 1, 0, "SimulationController")
           └─> Load main.qml
               └─> QML instantiates SimulationController
                   └─> Controller creates Simulator instance
                       └─> Controller creates ViewModels
                           └─> Connect signals: simulator->clockTicked() to viewModel->onClockTicked()
```

### 3.2 Flujo de un Tick de Simulación (Tiempo Real)

```
┌─────────────────────────────────────────────────────────────────┐
│ 1. USER ACTION                                                  │
│    User clicks "Step" button in ControlPanel.qml               │
└────────────┬────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 2. QML INVOKES CONTROLLER                                       │
│    onClicked: simulationController.step()                       │
└────────────┬────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 3. CONTROLLER CALLS SIMULATOR                                   │
│    SimulationController::step() { m_simulator->tick(); }        │
└────────────┬────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 4. SIMULATOR EXECUTES TICK                                      │
│    - handleArrivals()                                           │
│    - handleIO()                                                 │
│    - handlePageFaults()                                         │
│    - handleCpuExecution()                                       │
│    - handleScheduling()                                         │
│    - Update metrics                                             │
└────────────┬────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 5. SIMULATOR EMITS SIGNALS                                      │
│    emit clockTicked(m_clock.getTime());                         │
│    emit processStateChanged(pid, newState);  // Si hubo cambios │
└────────────┬────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 6. VIEWMODELS RECEIVE SIGNALS (Slots)                           │
│    ProcessMonitorViewModel::onClockTicked(uint64_t tick)        │
│    MemoryMonitorViewModel::onClockTicked(uint64_t tick)         │
│    BlockingEventsViewModel::onProcessStateChanged(pid, state)   │
└────────────┬────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 7. VIEWMODELS QUERY SIMULATOR (Getters)                         │
│    auto processes = m_simulator->getAllProcesses();             │
│    auto frames = m_memoryManager->getFrameStatus();             │
│    auto metrics = m_simulator->getSimulatorMetrics();           │
└────────────┬────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 8. VIEWMODELS UPDATE INTERNAL STATE                             │
│    - Create Qt wrappers (ProcessItemModel, FrameItemModel)      │
│    - Update Q_PROPERTY members                                  │
│    - Calculate derived values for UI                            │
└────────────┬────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 9. VIEWMODELS EMIT SIGNALS                                      │
│    emit processListChanged();                                   │
│    emit metricsChanged();                                       │
│    emit frameListChanged();                                     │
└────────────┬────────────────────────────────────────────────────┘
             │
             ▼
┌─────────────────────────────────────────────────────────────────┐
│ 10. QML REACTS TO CHANGES (Bindings)                            │
│     ListView detects model change → re-renders delegates        │
│     Text bindings update → displays new values                  │
│     Canvas receives repaint signal → redraws Gantt chart        │
└─────────────────────────────────────────────────────────────────┘
```

### 3.3 Flujo de Modo Automático (Play)

```
User clicks "Play"
  └─> ControlPanel.qml calls: simulationController.play()
      └─> SimulationController starts QTimer
          └─> QTimer::timeout() signal every N ms
              └─> Connected to SimulationController::onTimerTick()
                  └─> Calls m_simulator->tick()
                      └─> [Same flow as Step from point 4 onwards]
```

---

## 4. Componentes Principales

### 4.1 SimulationController

**Ubicación**: `src/gui/controllers/SimulationController.h`

**Responsabilidad**: Orquestar la simulación y controlar el timing.

```cpp
class SimulationController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int currentTick READ currentTick NOTIFY tickChanged)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningStateChanged)
    Q_PROPERTY(int speed READ speed WRITE setSpeed NOTIFY speedChanged)
    Q_PROPERTY(QString cpuStatus READ cpuStatus NOTIFY cpuStatusChanged)

public:
    explicit SimulationController(QObject* parent = nullptr);

    // Getters
    int currentTick() const;
    bool isRunning() const;
    int speed() const;  // milliseconds per tick
    QString cpuStatus() const;

    // Setters
    void setSpeed(int ms);

    // Simulator access (for ViewModels)
    Simulator* simulator() { return m_simulator.get(); }

public slots:
    Q_INVOKABLE void loadProcessesFromFile(const QString& filePath);
    Q_INVOKABLE void step();
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void reset();

signals:
    void tickChanged();
    void runningStateChanged();
    void speedChanged();
    void cpuStatusChanged();
    void errorOccurred(QString message);

private slots:
    void onTimerTick();
    void onSimulatorClockTicked(uint64_t tick);
    void updateCpuStatus();

private:
    std::unique_ptr<Simulator> m_simulator;
    QTimer* m_timer;
    int m_speed = 250;  // Default: 250ms per tick
};
```

### 4.2 ViewModels

#### ProcessMonitorViewModel

```cpp
class ProcessMonitorViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> processList READ processList NOTIFY processListChanged)
    Q_PROPERTY(QList<QObject*> readyQueue READ readyQueue NOTIFY readyQueueChanged)
    Q_PROPERTY(QList<QObject*> blockedQueue READ blockedQueue NOTIFY blockedQueueChanged)
    Q_PROPERTY(QObject* runningProcess READ runningProcess NOTIFY runningProcessChanged)
    Q_PROPERTY(double avgWaitTime READ avgWaitTime NOTIFY metricsChanged)
    Q_PROPERTY(double avgTurnaroundTime READ avgTurnaroundTime NOTIFY metricsChanged)
    Q_PROPERTY(double cpuUtilization READ cpuUtilization NOTIFY metricsChanged)
    Q_PROPERTY(QString schedulerAlgorithm READ schedulerAlgorithm NOTIFY algorithmChanged)

public:
    explicit ProcessMonitorViewModel(Simulator* simulator, QObject* parent = nullptr);

    // Property getters
    QList<QObject*> processList() const;
    QList<QObject*> readyQueue() const;
    QList<QObject*> blockedQueue() const;
    QObject* runningProcess() const;
    double avgWaitTime() const;
    double avgTurnaroundTime() const;
    double cpuUtilization() const;
    QString schedulerAlgorithm() const;

signals:
    void processListChanged();
    void readyQueueChanged();
    void blockedQueueChanged();
    void runningProcessChanged();
    void metricsChanged();
    void algorithmChanged();

public slots:
    void onClockTicked(uint64_t tick);
    void onProcessStateChanged(int pid, ProcessState state);

private:
    void updateAllData();
    QString formatState(ProcessState state) const;

    Simulator* m_simulator;
    QList<QObject*> m_processItems;
    QList<QObject*> m_readyQueueItems;
    QList<QObject*> m_blockedQueueItems;
    QObject* m_runningProcessItem = nullptr;
    double m_avgWaitTime = 0.0;
    double m_avgTurnaroundTime = 0.0;
    double m_cpuUtilization = 0.0;
    QString m_schedulerAlgorithm;
};
```

#### MemoryMonitorViewModel

```cpp
class MemoryMonitorViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> frameList READ frameList NOTIFY frameListChanged)
    Q_PROPERTY(QList<QObject*> pageTableList READ pageTableList NOTIFY pageTableListChanged)
    Q_PROPERTY(int totalFrames READ totalFrames NOTIFY configChanged)
    Q_PROPERTY(int usedFrames READ usedFrames NOTIFY frameListChanged)
    Q_PROPERTY(int totalPageFaults READ totalPageFaults NOTIFY statsChanged)
    Q_PROPERTY(int totalReplacements READ totalReplacements NOTIFY statsChanged)
    Q_PROPERTY(double hitRatio READ hitRatio NOTIFY statsChanged)
    Q_PROPERTY(QString memoryAlgorithm READ memoryAlgorithm NOTIFY algorithmChanged)

public:
    explicit MemoryMonitorViewModel(IMemoryManager* memoryManager, QObject* parent = nullptr);

    // Property getters
    QList<QObject*> frameList() const;
    QList<QObject*> pageTableList() const;
    int totalFrames() const;
    int usedFrames() const;
    int totalPageFaults() const;
    int totalReplacements() const;
    double hitRatio() const;
    QString memoryAlgorithm() const;

public slots:
    Q_INVOKABLE void selectProcess(int pid);  // Para mostrar page table de proceso específico
    void onClockTicked(uint64_t tick);

signals:
    void frameListChanged();
    void pageTableListChanged();
    void configChanged();
    void statsChanged();
    void algorithmChanged();

private:
    void updateFrameStatus();
    void updatePageTableForSelectedProcess();

    IMemoryManager* m_memoryManager;
    QList<QObject*> m_frameItems;
    QList<QObject*> m_pageTableItems;
    int m_selectedPid = -1;
    int m_totalFrames = 0;
    int m_totalPageFaults = 0;
    int m_totalReplacements = 0;
    double m_hitRatio = 0.0;
    QString m_memoryAlgorithm;
};
```

#### BlockingEventsViewModel

```cpp
struct BlockingEvent {
    uint64_t tick;
    int pid;
    QString eventType;  // "BLOCKED_IO", "BLOCKED_MEMORY", "UNBLOCKED"
    QString description;
};

class BlockingEventsViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QList<QObject*> eventList READ eventList NOTIFY eventListChanged)
    Q_PROPERTY(int blockedByIOCount READ blockedByIOCount NOTIFY countsChanged)
    Q_PROPERTY(int blockedByMemoryCount READ blockedByMemoryCount NOTIFY countsChanged)

public:
    explicit BlockingEventsViewModel(Simulator* simulator, QObject* parent = nullptr);

    QList<QObject*> eventList() const;
    int blockedByIOCount() const;
    int blockedByMemoryCount() const;

signals:
    void eventListChanged();
    void countsChanged();

public slots:
    void onProcessStateChanged(int pid, ProcessState oldState, ProcessState newState);
    void onClockTicked(uint64_t tick);
    Q_INVOKABLE void clearEvents();

private:
    void addEvent(uint64_t tick, int pid, const QString& type, const QString& desc);

    Simulator* m_simulator;
    QList<QObject*> m_eventItems;
    uint64_t m_currentTick = 0;
};
```

### 4.3 Item Models (Wrappers Qt para QML)

Estos objetos wrapper permiten exponer datos C++ a QML:

```cpp
class ProcessItemModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int pid READ pid CONSTANT)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(int priority READ priority CONSTANT)
    Q_PROPERTY(uint64_t waitTime READ waitTime NOTIFY statsChanged)
    Q_PROPERTY(uint64_t cpuTime READ cpuTime NOTIFY statsChanged)
    Q_PROPERTY(QString currentBurst READ currentBurst NOTIFY burstChanged)
    Q_PROPERTY(int pageFaults READ pageFaults NOTIFY statsChanged)

public:
    int pid() const { return m_pid; }
    QString state() const { return m_state; }
    // ... otros getters

    void updateFrom(const Process* process);

signals:
    void stateChanged();
    void statsChanged();
    void burstChanged();

private:
    int m_pid;
    QString m_state;
    int m_priority;
    uint64_t m_waitTime;
    uint64_t m_cpuTime;
    QString m_currentBurst;
    int m_pageFaults;
};
```

---

## 5. Threading y Sincronización

### 5.1 Modelo de Threading Adoptado

**Decisión**: Todo se ejecuta en el **UI thread** (main thread de Qt).

**Justificación**:

- La simulación es determinística y controlada tick-a-tick
- No hay operaciones bloqueantes (I/O real, cálculos pesados)
- Simplifica enormemente la sincronización
- Los ticks se ejecutan en < 1ms típicamente

### 5.2 Flujo de Ejecución

```
UI Thread:
  └─> QTimer::timeout() (cada N ms)
      └─> SimulationController::onTimerTick()
          └─> Simulator::tick()  [Mutex locked internamente]
              └─> emit clockTicked()
                  └─> ViewModels::onClockTicked() [Mismo thread]
                      └─> Simulator getters [Mutex locked]
                          └─> emit property changed signals
                              └─> QML updates [Mismo thread]
```

### 5.3 Thread Safety en Backend

Aunque todo corre en UI thread, el backend mantiene locks por:

1. **Buenas prácticas**: Preparado para futuro multithreading
2. **Consistencia**: Evita estados intermedios durante tick()

```cpp
// En Simulator.cpp
std::vector<const Process*> Simulator::getAllProcesses() const {
    std::lock_guard<std::mutex> lock(m_simulationMutex);
    // ... acceso seguro a m_processes
}
```

### 5.4 Prevención de Re-entrancia

**Problema**: Si `tick()` emite signal que trigger otro `tick()`.

**Solución**: Flag de control en `SimulationController`:

```cpp
void SimulationController::step() {
    if (m_isExecutingTick) return;  // Prevenir re-entrancia

    m_isExecutingTick = true;
    m_simulator->tick();
    m_isExecutingTick = false;
}
```

---

## 6. Transición de Mock a Datos Reales

### 6.1 Fase 1: Solo Mock (Sprint 1)

```cpp
// En ProcessMonitorViewModel
void ProcessMonitorViewModel::onClockTicked(uint64_t tick) {
    if (m_useMockData) {
        // Usar MockDataProvider
        auto mockProcesses = MockDataProvider::generateProcessList(tick);
        updateFromMockData(mockProcesses);
    }
}
```

### 6.2 Fase 2: Integración Parcial (Sprint 2)

```cpp
void ProcessMonitorViewModel::onClockTicked(uint64_t tick) {
    if (m_simulator && !m_useMockData) {
        // Usar datos reales del Simulator
        auto realProcesses = m_simulator->getAllProcesses();
        updateFromRealData(realProcesses);
    } else {
        // Fallback a mock si backend no disponible
        auto mockProcesses = MockDataProvider::generateProcessList(tick);
        updateFromMockData(mockProcesses);
    }
}
```

### 6.3 Fase 3: Solo Datos Reales (Sprint 3)

```cpp
void ProcessMonitorViewModel::onClockTicked(uint64_t tick) {
    Q_ASSERT(m_simulator != nullptr);  // Garantizar que está disponible

    auto processes = m_simulator->getAllProcesses();
    updateFromRealData(processes);
}
```

### 6.4 Estrategia de Interfaz Abstracta (Opcional)

Para facilitar testing y transición gradual:

```cpp
// IDataSource.h
class ISimulatorDataSource {
public:
    virtual ~ISimulatorDataSource() = default;
    virtual std::vector<const Process*> getAllProcesses() const = 0;
    virtual const Process* getRunningProcess() const = 0;
    // ... otros métodos
};

// MockDataSource.h
class MockDataSource : public ISimulatorDataSource {
    std::vector<const Process*> getAllProcesses() const override {
        return MockDataProvider::generateProcessList();
    }
};

// RealSimulatorDataSource.h
class RealSimulatorDataSource : public ISimulatorDataSource {
public:
    RealSimulatorDataSource(Simulator* sim) : m_sim(sim) {}

    std::vector<const Process*> getAllProcesses() const override {
        return m_sim->getAllProcesses();
    }

private:
    Simulator* m_sim;
};

// En ViewModel
ProcessMonitorViewModel::ProcessMonitorViewModel(ISimulatorDataSource* source)
    : m_dataSource(source) {
    // Funciona con mock o real transparentemente
}
```

---

## 📝 Resumen de Decisiones Arquitectónicas

| Decisión                 | Razón                                               |
| ------------------------ | --------------------------------------------------- |
| **MVVM Pattern**         | Separación clara de responsabilidades, testabilidad |
| **Qt Quick (QML)**       | Desarrollo rápido de UI, animaciones fluidas        |
| **Single Thread**        | Simplicidad, evitar race conditions                 |
| **Properties + Signals** | Binding reactivo automático de Qt                   |
| **Mock First**           | Desarrollo paralelo GUI/Backend                     |
| **Const Getters**        | APIs observadoras no invasivas                      |
| **Shared Structs**       | Evitar duplicación entre GUI/Backend                |

---

**Última actualización**: 24 de noviembre de 2025  
**Versión**: 1.0  
**Contacto**: Issue #24 en GitHub
