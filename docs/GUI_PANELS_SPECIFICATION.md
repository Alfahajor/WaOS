# GUI Panels Specification

**Especificación detallada de paneles visuales para el Monitor del Sistema Operativo WaOS**

Este documento mapea cada requisito de las especificaciones de entrega a componentes QML concretos, definiendo layouts, componentes visuales y datos mostrados.

---

## 📋 Índice

1. [Panel de Planificación](#1-panel-de-planificación)
2. [Panel de Memoria](#2-panel-de-memoria)
3. [Panel de Bloqueos y Sincronización](#3-panel-de-bloqueos-y-sincronización)
4. [Panel de Control](#4-panel-de-control)
5. [Diagrama de Gantt](#5-diagrama-de-gantt)
6. [Mapeo de Especificaciones](#6-mapeo-de-especificaciones)

---

## 1. Panel de Planificación

**Archivo**: `src/gui/qml/ProcessMonitor.qml`

### 1.1 Especificación de Entrega Cumplida

> **a) Planificación**
>
> - Estado de las colas de procesos (Listo, Bloqueado, Ejecutando).
> - Algoritmo activo.
> - Diagrama de Gantt o log de ejecución.
> - Métricas: Tiempo promedio de espera, Tiempo promedio de retorno, Utilización de CPU.

### 1.2 Layout del Panel

```
┌─────────────────────────────────────────────────────────────────┐
│ PANEL DE PLANIFICACIÓN                                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│ Algoritmo Activo: [FCFS (First-Come-First-Served)]             │
│                                                                  │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ PROCESO EN CPU (Ejecutando)                                 │ │
│ │ ┌───────────────────────────────────────────────────────┐   │ │
│ │ │ P1 | Prioridad: 1 | Burst: CPU(3) | Tiempo CPU: 15   │   │ │
│ │ └───────────────────────────────────────────────────────┘   │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ COLA READY (Listos)                             [5 procesos]│ │
│ │ ┌──────┬───────────┬──────────┬────────────┬──────────────┐ │ │
│ │ │ PID  │ Prioridad │ T.Espera │ Burst Sig  │ Page Faults  │ │ │
│ │ ├──────┼───────────┼──────────┼────────────┼──────────────┤ │ │
│ │ │  P2  │     2     │    45    │   CPU(8)   │      3       │ │ │
│ │ │  P3  │     1     │    30    │   CPU(5)   │      1       │ │ │
│ │ │  P5  │     3     │    12    │   CPU(2)   │      0       │ │ │
│ │ └──────┴───────────┴──────────┴────────────┴──────────────┘ │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ PROCESOS BLOQUEADOS                             [2 procesos]│ │
│ │ ┌──────┬───────────────┬────────────┬───────────────────────┐│
│ │ │ PID  │ Motivo        │ Tiempo Bloq│ Info                  ││
│ │ ├──────┼───────────────┼────────────┼───────────────────────┤│
│ │ │  P4  │ E/S           │     3      │ Duración E/S: 5 ticks ││
│ │ │  P6  │ Page Fault    │     2      │ Página 3, falta: 3t   ││
│ │ └──────┴───────────────┴────────────┴───────────────────────┘│
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ MÉTRICAS GLOBALES                                           │ │
│ │  Tiempo Promedio Espera:     32.5 ticks                     │ │
│ │  Tiempo Promedio Retorno:    85.3 ticks                     │ │
│ │  Utilización CPU:            78.2%                          │ │
│ │  Context Switches:           24                             │ │
│ │  Procesos Completados:       3 / 8                          │ │
│ └─────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

### 1.3 Componentes QML

```qml
// ProcessMonitor.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import WaOS 1.0

Rectangle {
    id: root
    color: "#F5F5F5"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Header: Algoritmo Activo
        Text {
            text: "Algoritmo Activo: " + processViewModel.schedulerAlgorithm
            font.pixelSize: 16
            font.bold: true
        }

        // Proceso en CPU
        Rectangle {
            Layout.fillWidth: true
            height: 80
            color: "#E8F5E9"
            border.color: "#4CAF50"
            border.width: 2
            radius: 5

            RowLayout {
                anchors.centerIn: parent
                spacing: 20

                Text {
                    text: "PROCESO EN CPU:"
                    font.bold: true
                }

                Text {
                    text: processViewModel.runningProcess ?
                          formatRunningProcess(processViewModel.runningProcess) :
                          "CPU Idle"
                    font.pixelSize: 14
                }
            }
        }

        // Cola Ready
        GroupBox {
            title: "COLA READY (" + processViewModel.readyQueue.length + " procesos)"
            Layout.fillWidth: true
            Layout.fillHeight: true

            TableView {
                id: readyTableView
                anchors.fill: parent
                model: processViewModel.readyQueue

                delegate: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 35
                    color: index % 2 === 0 ? "#FFF9C4" : "#FFFDE7"
                    border.color: "#FBC02D"

                    Text {
                        anchors.centerIn: parent
                        text: getCellText(column, modelData)
                    }
                }
            }
        }

        // Procesos Bloqueados
        GroupBox {
            title: "PROCESOS BLOQUEADOS (" + processViewModel.blockedQueue.length + " procesos)"
            Layout.fillWidth: true
            Layout.preferredHeight: 150

            ListView {
                anchors.fill: parent
                model: processViewModel.blockedQueue
                spacing: 5

                delegate: Rectangle {
                    width: parent.width
                    height: 40
                    color: modelData.blockReason === "IO" ? "#FFCDD2" : "#FFE0B2"
                    border.color: modelData.blockReason === "IO" ? "#F44336" : "#FF9800"
                    radius: 3

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5

                        Text {
                            text: "P" + modelData.pid
                            font.bold: true
                        }
                        Text { text: " | Motivo: " + modelData.blockReason }
                        Text { text: " | Tiempo bloqueado: " + modelData.blockedDuration + " ticks" }
                    }
                }
            }
        }

        // Métricas
        GroupBox {
            title: "MÉTRICAS GLOBALES"
            Layout.fillWidth: true
            Layout.preferredHeight: 120

            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 20
                rowSpacing: 5

                Text { text: "Tiempo Promedio Espera:" }
                Text {
                    text: processViewModel.avgWaitTime.toFixed(1) + " ticks"
                    font.bold: true
                    color: "#1976D2"
                }

                Text { text: "Tiempo Promedio Retorno:" }
                Text {
                    text: processViewModel.avgTurnaroundTime.toFixed(1) + " ticks"
                    font.bold: true
                    color: "#1976D2"
                }

                Text { text: "Utilización CPU:" }
                Text {
                    text: processViewModel.cpuUtilization.toFixed(1) + "%"
                    font.bold: true
                    color: "#388E3C"
                }

                Text { text: "Context Switches:" }
                Text {
                    text: processViewModel.contextSwitches
                    font.bold: true
                }
            }
        }
    }

    function formatRunningProcess(proc) {
        return "P" + proc.pid +
               " | Prioridad: " + proc.priority +
               " | Burst: " + proc.currentBurst +
               " | Tiempo CPU: " + proc.cpuTime
    }
}
```

### 1.4 Datos Requeridos del ViewModel

```cpp
// ProcessMonitorViewModel debe exponer:
Q_PROPERTY(QList<QObject*> readyQueue ...)           // Cola Ready
Q_PROPERTY(QList<QObject*> blockedQueue ...)         // Cola Blocked
Q_PROPERTY(QObject* runningProcess ...)              // Proceso en CPU
Q_PROPERTY(double avgWaitTime ...)                   // Tiempo promedio espera
Q_PROPERTY(double avgTurnaroundTime ...)             // Tiempo promedio retorno
Q_PROPERTY(double cpuUtilization ...)                // Utilización CPU %
Q_PROPERTY(QString schedulerAlgorithm ...)           // Nombre del algoritmo
Q_PROPERTY(int contextSwitches ...)                  // Total context switches
```

---

## 2. Panel de Memoria

**Archivo**: `src/gui/qml/MemoryMonitor.qml`

### 2.1 Especificación de Entrega Cumplida

> **b) Memoria**
>
> - Tabla de páginas por proceso.
> - Estado de los marcos de memoria (ocupado/libre).
> - Cantidad total de fallos de página y reemplazos realizados.

### 2.2 Layout del Panel

```
┌─────────────────────────────────────────────────────────────────┐
│ PANEL DE MEMORIA                                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│ Algoritmo: [LRU (Least Recently Used)]  Marcos: 16  Usados: 12 │
│                                                                  │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ MEMORIA FÍSICA (Frames)                                     │ │
│ │                                                             │ │
│ │  ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐  │ │
│ │  │P1:0│ │P1:1│ │P2:0│ │P3:2│ │────│ │P2:1│ │P4:0│ │────│  │ │
│ │  │ 0  │ │ 1  │ │ 2  │ │ 3  │ │ 4  │ │ 5  │ │ 6  │ │ 7  │  │ │
│ │  └────┘ └────┘ └────┘ └────┘ └────┘ └────┘ └────┘ └────┘  │ │
│ │                                                             │ │
│ │  ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐  │ │
│ │  │P1:2│ │P3:0│ │────│ │P4:1│ │P2:3│ │────│ │P3:1│ │────│  │ │
│ │  │ 8  │ │ 9  │ │ 10 │ │ 11 │ │ 12 │ │ 13 │ │ 14 │ │ 15 │  │ │
│ │  └────┘ └────┘ └────┘ └────┘ └────┘ └────┘ └────┘ └────┘  │ │
│ │                                                             │ │
│ │  Verde: Ocupado  |  Gris: Libre                            │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ TABLAS DE PÁGINAS POR PROCESO                               │ │
│ │                                                             │ │
│ │ Seleccionar Proceso: [▼ P1 ▼]                              │ │
│ │                                                             │ │
│ │ ┌────────┬──────────┬─────────┬───────────┬──────────────┐ │ │
│ │ │ Página │ Frame    │ Presente│ Referenc. │ Modificado   │ │ │
│ │ ├────────┼──────────┼─────────┼───────────┼──────────────┤ │ │
│ │ │   0    │    0     │    ✓    │     ✓     │      ✗       │ │ │
│ │ │   1    │    1     │    ✓    │     ✓     │      ✓       │ │ │
│ │ │   2    │    8     │    ✓    │     ✗     │      ✗       │ │ │
│ │ │   3    │   ---    │    ✗    │     ✗     │      ✗       │ │ │
│ │ └────────┴──────────┴─────────┴───────────┴──────────────┘ │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ ESTADÍSTICAS DE MEMORIA                                     │ │
│ │  Total Page Faults:       47                                │ │
│ │  Total Reemplazos:        12                                │ │
│ │  Hit Ratio:               73.5%                             │ │
│ │  Frames Ocupados:         12 / 16                           │ │
│ └─────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

### 2.3 Componentes QML

```qml
// MemoryMonitor.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "#F5F5F5"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Header
        RowLayout {
            Text {
                text: "Algoritmo: " + memoryViewModel.memoryAlgorithm
                font.bold: true
            }
            Text {
                text: " | Marcos: " + memoryViewModel.totalFrames
            }
            Text {
                text: " | Usados: " + memoryViewModel.usedFrames
                color: "#388E3C"
                font.bold: true
            }
        }

        // Memoria Física (Grid de Frames)
        GroupBox {
            title: "MEMORIA FÍSICA (Frames)"
            Layout.fillWidth: true
            Layout.preferredHeight: 250

            GridView {
                id: frameGrid
                anchors.fill: parent
                cellWidth: 70
                cellHeight: 70
                model: memoryViewModel.frameList

                delegate: Rectangle {
                    width: 65
                    height: 65
                    color: modelData.occupied ? "#4CAF50" : "#9E9E9E"
                    border.color: "#212121"
                    border.width: 1
                    radius: 3

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: modelData.label
                            color: "white"
                            font.bold: true
                            font.pixelSize: 10
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: modelData.frameId
                            color: "white"
                            font.pixelSize: 9
                        }
                    }

                    ToolTip.visible: frameMouseArea.containsMouse
                    ToolTip.text: modelData.occupied ?
                        "Frame " + modelData.frameId +
                        "\nProceso: P" + modelData.ownerPid +
                        "\nPágina: " + modelData.pageNumber +
                        "\nCargado en tick: " + modelData.loadedAtTick :
                        "Frame " + modelData.frameId + " (Libre)"

                    MouseArea {
                        id: frameMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                    }
                }
            }

            // Leyenda
            Row {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.margins: 5
                spacing: 15

                Rectangle { width: 20; height: 20; color: "#4CAF50" }
                Text { text: "Ocupado"; anchors.verticalCenter: parent.verticalCenter }

                Rectangle { width: 20; height: 20; color: "#9E9E9E" }
                Text { text: "Libre"; anchors.verticalCenter: parent.verticalCenter }
            }
        }

        // Tabla de Páginas
        GroupBox {
            title: "TABLA DE PÁGINAS POR PROCESO"
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 5

                ComboBox {
                    id: processSelector
                    Layout.fillWidth: true
                    model: memoryViewModel.activeProcesses  // Lista de PIDs
                    displayText: "Proceso: P" + currentValue

                    onCurrentValueChanged: {
                        memoryViewModel.selectProcess(currentValue)
                    }
                }

                TableView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: memoryViewModel.pageTableList

                    delegate: Rectangle {
                        implicitWidth: 100
                        implicitHeight: 30
                        color: modelData.present ? "#E8F5E9" : "#FFEBEE"
                        border.color: "#BDBDBD"

                        Text {
                            anchors.centerIn: parent
                            text: getPageTableCellText(column, modelData)
                        }
                    }
                }
            }
        }

        // Estadísticas
        GroupBox {
            title: "ESTADÍSTICAS DE MEMORIA"
            Layout.fillWidth: true
            Layout.preferredHeight: 100

            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 20

                Text { text: "Total Page Faults:" }
                Text {
                    text: memoryViewModel.totalPageFaults
                    font.bold: true
                    color: "#D32F2F"
                }

                Text { text: "Total Reemplazos:" }
                Text {
                    text: memoryViewModel.totalReplacements
                    font.bold: true
                }

                Text { text: "Hit Ratio:" }
                Text {
                    text: memoryViewModel.hitRatio.toFixed(1) + "%"
                    font.bold: true
                    color: "#388E3C"
                }
            }
        }
    }
}
```

### 2.4 Datos Requeridos del ViewModel

```cpp
Q_PROPERTY(QList<QObject*> frameList ...)            // Lista de frames
Q_PROPERTY(QList<QObject*> pageTableList ...)        // Tabla de páginas del proceso seleccionado
Q_PROPERTY(QList<int> activeProcesses ...)           // PIDs de procesos activos
Q_PROPERTY(int totalFrames ...)                      // Total de frames
Q_PROPERTY(int usedFrames ...)                       // Frames ocupados
Q_PROPERTY(int totalPageFaults ...)                  // Page faults totales
Q_PROPERTY(int totalReplacements ...)                // Reemplazos totales
Q_PROPERTY(double hitRatio ...)                      // Hit ratio %
Q_PROPERTY(QString memoryAlgorithm ...)              // Nombre del algoritmo

Q_INVOKABLE void selectProcess(int pid);             // Cambiar proceso mostrado
```

---

## 3. Panel de Bloqueos y Sincronización

**Archivo**: `src/gui/qml/BlockingPanel.qml`

### 3.1 Especificación de Entrega Cumplida

> **c) Bloqueos y sincronización**
>
> - Indicadores de procesos bloqueados por memoria o por E/S.
> - Notificaciones cuando un proceso pasa de "bloqueado" a "listo"

### 3.2 Layout del Panel

```
┌─────────────────────────────────────────────────────────────────┐
│ PANEL DE BLOQUEOS Y SINCRONIZACIÓN                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ CONTADORES                                                  │ │
│ │  Bloqueados por E/S:      2 procesos                        │ │
│ │  Bloqueados por Memoria:  1 proceso                         │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ LOG DE EVENTOS DE BLOQUEO                      [Limpiar Log]│ │
│ │                                                             │ │
│ │ ┌─────────────────────────────────────────────────────────┐ │ │
│ │ │ [Tick 45] ⏸ P4 → BLOQUEADO por E/S (Duración: 5 ticks) │ │ │
│ │ │ [Tick 47] ⚠ P6 → BLOQUEADO por Page Fault (Pág. 3)     │ │ │
│ │ │ [Tick 50] ✓ P4 → DESBLOQUEADO (E/S completada)         │ │ │
│ │ │ [Tick 52] ✓ P6 → DESBLOQUEADO (Página cargada)         │ │ │
│ │ │ [Tick 53] ⏸ P2 → BLOQUEADO por E/S (Duración: 3 ticks) │ │ │
│ │ │ [Tick 55] ⚠ P1 → BLOQUEADO por Page Fault (Pág. 7)     │ │ │
│ │ └─────────────────────────────────────────────────────────┘ │ │
│ │                                                             │ │
│ │ Total eventos: 24                                           │ │
│ └─────────────────────────────────────────────────────────────┘ │
│                                                                  │
│ ┌─────────────────────────────────────────────────────────────┐ │
│ │ PROCESOS ACTUALMENTE BLOQUEADOS                             │ │
│ │                                                             │ │
│ │  🔴 P2 - Bloqueado por E/S                                  │ │
│ │     Tick inicio: 53 | Duración esperada: 3 | Restante: 1   │ │
│ │                                                             │ │
│ │  🟠 P1 - Bloqueado por Page Fault                           │ │
│ │     Tick inicio: 55 | Página: 7 | Penalty restante: 4      │ │
│ │                                                             │ │
│ └─────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

### 3.3 Componentes QML

```qml
// BlockingPanel.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    color: "#F5F5F5"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Contadores
        GroupBox {
            title: "CONTADORES"
            Layout.fillWidth: true
            Layout.preferredHeight: 80

            GridLayout {
                anchors.fill: parent
                columns: 2

                Text { text: "Bloqueados por E/S:" }
                Text {
                    text: blockingViewModel.blockedByIOCount + " procesos"
                    font.bold: true
                    color: "#F44336"
                }

                Text { text: "Bloqueados por Memoria:" }
                Text {
                    text: blockingViewModel.blockedByMemoryCount + " procesos"
                    font.bold: true
                    color: "#FF9800"
                }
            }
        }

        // Log de Eventos
        GroupBox {
            title: "LOG DE EVENTOS DE BLOQUEO"
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 5

                Button {
                    text: "Limpiar Log"
                    Layout.alignment: Qt.AlignRight
                    onClicked: blockingViewModel.clearEvents()
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ListView {
                        id: eventListView
                        model: blockingViewModel.eventList
                        spacing: 3

                        delegate: Rectangle {
                            width: parent.width
                            height: 35
                            color: getEventColor(modelData.eventType)
                            border.color: "#757575"
                            radius: 3

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 5
                                spacing: 10

                                Text {
                                    text: "[Tick " + modelData.tick + "]"
                                    font.bold: true
                                }

                                Text {
                                    text: getEventIcon(modelData.eventType)
                                    font.pixelSize: 16
                                }

                                Text {
                                    text: modelData.description
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }
                }

                Text {
                    text: "Total eventos: " + blockingViewModel.eventList.length
                    font.italic: true
                }
            }
        }

        // Procesos Actualmente Bloqueados
        GroupBox {
            title: "PROCESOS ACTUALMENTE BLOQUEADOS"
            Layout.fillWidth: true
            Layout.preferredHeight: 150

            ListView {
                anchors.fill: parent
                model: blockingViewModel.currentlyBlockedProcesses
                spacing: 10

                delegate: Rectangle {
                    width: parent.width
                    height: 60
                    color: modelData.blockType === "IO" ? "#FFCDD2" : "#FFE0B2"
                    border.color: modelData.blockType === "IO" ? "#F44336" : "#FF9800"
                    border.width: 2
                    radius: 5

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8

                        Text {
                            text: (modelData.blockType === "IO" ? "🔴" : "🟠") +
                                  " P" + modelData.pid + " - Bloqueado por " +
                                  (modelData.blockType === "IO" ? "E/S" : "Page Fault")
                            font.bold: true
                            font.pixelSize: 12
                        }

                        Text {
                            text: "Tick inicio: " + modelData.startTick +
                                  " | " + (modelData.blockType === "IO" ?
                                      "Duración esperada: " + modelData.duration +
                                      " | Restante: " + modelData.remaining :
                                      "Página: " + modelData.pageNumber +
                                      " | Penalty restante: " + modelData.remaining)
                            font.pixelSize: 10
                            color: "#424242"
                        }
                    }
                }
            }
        }
    }

    function getEventIcon(eventType) {
        switch(eventType) {
            case "BLOCKED_IO": return "⏸"
            case "BLOCKED_MEMORY": return "⚠"
            case "UNBLOCKED": return "✓"
            default: return "•"
        }
    }

    function getEventColor(eventType) {
        switch(eventType) {
            case "BLOCKED_IO": return "#FFEBEE"
            case "BLOCKED_MEMORY": return "#FFF3E0"
            case "UNBLOCKED": return "#E8F5E9"
            default: return "#FFFFFF"
        }
    }
}
```

### 3.4 Datos Requeridos del ViewModel

```cpp
Q_PROPERTY(QList<QObject*> eventList ...)                    // Log de eventos
Q_PROPERTY(QList<QObject*> currentlyBlockedProcesses ...)    // Procesos bloqueados ahora
Q_PROPERTY(int blockedByIOCount ...)                         // Contador E/S
Q_PROPERTY(int blockedByMemoryCount ...)                     // Contador Page Fault

Q_INVOKABLE void clearEvents();                              // Limpiar log
```

---

## 4. Panel de Control

**Archivo**: `src/gui/qml/ControlPanel.qml`

### 4.1 Layout del Panel

```
┌─────────────────────────────────────────────────────────────────┐
│ CONTROL DE SIMULACIÓN                                           │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐           │
│  │  ▶ Play │  │ ⏸ Pause │  │ ⏭ Step  │  │ ⟲ Reset │           │
│  └─────────┘  └─────────┘  └─────────┘  └─────────┘           │
│                                                                  │
│  Velocidad: [────●──────────] 250 ms/tick                       │
│             Lento          Rápido                               │
│                                                                  │
│  Tick Actual: 127  |  Estado: Ejecutando  |  CPU: P3           │
│                                                                  │
│  Cargar Procesos: [📁 Seleccionar archivo...]                   │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### 4.2 Componentes QML

```qml
// ControlPanel.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs

Rectangle {
    color: "#ECEFF1"
    border.color: "#90A4AE"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // Botones de control
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 10

            Button {
                text: "▶ Play"
                enabled: !simulationController.isRunning
                onClicked: simulationController.play()
            }

            Button {
                text: "⏸ Pause"
                enabled: simulationController.isRunning
                onClicked: simulationController.pause()
            }

            Button {
                text: "⏭ Step"
                enabled: !simulationController.isRunning
                onClicked: simulationController.step()
            }

            Button {
                text: "⟲ Reset"
                onClicked: simulationController.reset()
            }
        }

        // Control de velocidad
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5

            Text {
                text: "Velocidad: " + simulationController.speed + " ms/tick"
                font.bold: true
            }

            Slider {
                Layout.fillWidth: true
                from: 50
                to: 1000
                value: simulationController.speed
                stepSize: 50

                onValueChanged: {
                    simulationController.speed = value
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Text { text: "Lento" }
                Item { Layout.fillWidth: true }
                Text { text: "Rápido" }
            }
        }

        // Información de estado
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: "#CFD8DC"
            border.color: "#78909C"
            radius: 3

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 15

                Text {
                    text: "Tick Actual: " + simulationController.currentTick
                    font.bold: true
                }

                Rectangle { width: 2; height: 20; color: "#78909C" }

                Text {
                    text: "Estado: " + (simulationController.isRunning ? "Ejecutando" : "Pausado")
                    color: simulationController.isRunning ? "#388E3C" : "#F57C00"
                    font.bold: true
                }

                Rectangle { width: 2; height: 20; color: "#78909C" }

                Text {
                    text: "CPU: " + simulationController.cpuStatus
                }
            }
        }

        // Carga de archivo
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "Cargar Procesos:"
            }

            Button {
                text: "📁 Seleccionar archivo..."
                onClicked: fileDialog.open()
            }
        }

        Item { Layout.fillHeight: true }
    }

    FileDialog {
        id: fileDialog
        title: "Seleccionar archivo de procesos"
        nameFilters: ["Archivos de texto (*.txt)"]
        onAccepted: {
            simulationController.loadProcessesFromFile(fileUrl)
        }
    }
}
```

---

## 5. Diagrama de Gantt

**Archivo**: `src/gui/qml/GanttChart.qml`

### 5.1 Visualización

```
┌─────────────────────────────────────────────────────────────────┐
│ DIAGRAMA DE GANTT                                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│ P1 │██│▓▓│  │██│██│  │▓▓│  │  │                                │
│ P2 │  │  │██│  │  │██│  │▓▓│  │██│                              │
│ P3 │  │  │  │  │  │  │██│  │██│  │██│                           │
│ P4 │  │██│  │▓▓│▓▓│▓▓│  │  │  │  │  │██│                        │
│    └──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──┴──                     │
│      0  5 10 15 20 25 30 35 40 45 50 55 60  Ticks              │
│                                                                  │
│  ██ RUNNING  ▓▓ BLOCKED  ░░ READY  ⬜ WAITING_MEMORY           │
└─────────────────────────────────────────────────────────────────┘
```

### 5.2 Implementación con Canvas

```qml
// GanttChart.qml
import QtQuick 2.15

Canvas {
    id: ganttCanvas
    width: parent.width
    height: 300

    property var ganttData: ganttViewModel.ganttEvents  // [{pid, tick, state}, ...]
    property int tickWidth: 10  // pixels por tick
    property int processHeight: 30

    onGanttDataChanged: {
        requestPaint()
    }

    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)

        // Dibujar grid de fondo
        drawGrid(ctx)

        // Dibujar barras de procesos
        drawProcessBars(ctx)

        // Dibujar leyenda
        drawLegend(ctx)
    }

    function drawProcessBars(ctx) {
        var processes = {}  // Agrupar eventos por PID

        for (var i = 0; i < ganttData.length; i++) {
            var event = ganttData[i]
            if (!processes[event.pid]) {
                processes[event.pid] = []
            }
            processes[event.pid].push(event)
        }

        var yOffset = 30
        for (var pid in processes) {
            var events = processes[pid]

            // Dibujar label del proceso
            ctx.fillStyle = "#000000"
            ctx.font = "12px sans-serif"
            ctx.fillText("P" + pid, 5, yOffset + 15)

            // Dibujar eventos del proceso
            for (var j = 0; j < events.length; j++) {
                var ev = events[j]
                var x = 50 + (ev.tick * tickWidth)
                var color = getStateColor(ev.state)

                ctx.fillStyle = color
                ctx.fillRect(x, yOffset, tickWidth, processHeight)
                ctx.strokeStyle = "#000000"
                ctx.strokeRect(x, yOffset, tickWidth, processHeight)
            }

            yOffset += processHeight + 5
        }
    }

    function getStateColor(state) {
        switch(state) {
            case "RUNNING": return "#4CAF50"        // Verde
            case "READY": return "#FFC107"          // Amarillo
            case "BLOCKED": return "#F44336"        // Rojo
            case "WAITING_MEMORY": return "#FF9800" // Naranja
            default: return "#9E9E9E"               // Gris
        }
    }

    function drawGrid(ctx) {
        ctx.strokeStyle = "#E0E0E0"
        ctx.lineWidth = 1

        // Líneas verticales (cada 5 ticks)
        for (var tick = 0; tick < 100; tick += 5) {
            var x = 50 + (tick * tickWidth)
            ctx.beginPath()
            ctx.moveTo(x, 0)
            ctx.lineTo(x, height - 30)
            ctx.stroke()

            // Labels de tick
            ctx.fillStyle = "#000000"
            ctx.font = "10px sans-serif"
            ctx.fillText(tick, x - 5, height - 10)
        }
    }

    function drawLegend(ctx) {
        var legendY = 10
        var legendX = width - 200

        drawLegendItem(ctx, legendX, legendY, "#4CAF50", "RUNNING")
        drawLegendItem(ctx, legendX, legendY + 20, "#F44336", "BLOCKED")
        drawLegendItem(ctx, legendX, legendY + 40, "#FFC107", "READY")
        drawLegendItem(ctx, legendX, legendY + 60, "#FF9800", "WAITING_MEMORY")
    }

    function drawLegendItem(ctx, x, y, color, label) {
        ctx.fillStyle = color
        ctx.fillRect(x, y, 15, 15)
        ctx.strokeStyle = "#000000"
        ctx.strokeRect(x, y, 15, 15)

        ctx.fillStyle = "#000000"
        ctx.font = "10px sans-serif"
        ctx.fillText(label, x + 20, y + 12)
    }
}
```

---

## 6. Mapeo de Especificaciones

### 6.1 Tabla de Cumplimiento

| Requisito de Entrega                               | Componente QML                              | ViewModel                                                        | API Backend                                                                      |
| -------------------------------------------------- | ------------------------------------------- | ---------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| **Estado de colas (Listo, Bloqueado, Ejecutando)** | `ProcessMonitor.qml` → TableView + ListView | `ProcessMonitorViewModel.readyQueue/blockedQueue/runningProcess` | `Simulator::getReadyProcesses()`, `getBlockedProcesses()`, `getRunningProcess()` |
| **Algoritmo activo**                               | `ProcessMonitor.qml` → Text header          | `ProcessMonitorViewModel.schedulerAlgorithm`                     | `Simulator::getSchedulerAlgorithmName()`                                         |
| **Diagrama de Gantt**                              | `GanttChart.qml` → Canvas                   | `GanttViewModel.ganttEvents`                                     | Acumulado desde signals `processStateChanged`                                    |
| **Tiempo promedio espera**                         | `ProcessMonitor.qml` → Métricas GroupBox    | `ProcessMonitorViewModel.avgWaitTime`                            | `SimulatorMetrics.avgWaitTime`                                                   |
| **Tiempo promedio retorno**                        | `ProcessMonitor.qml` → Métricas GroupBox    | `ProcessMonitorViewModel.avgTurnaroundTime`                      | `SimulatorMetrics.avgTurnaroundTime`                                             |
| **Utilización CPU**                                | `ProcessMonitor.qml` → Métricas GroupBox    | `ProcessMonitorViewModel.cpuUtilization`                         | `SimulatorMetrics.cpuUtilization`                                                |
| **Tabla de páginas por proceso**                   | `MemoryMonitor.qml` → TableView             | `MemoryMonitorViewModel.pageTableList`                           | `IMemoryManager::getPageTableForProcess(pid)`                                    |
| **Estado marcos (ocupado/libre)**                  | `MemoryMonitor.qml` → GridView              | `MemoryMonitorViewModel.frameList`                               | `IMemoryManager::getFrameStatus()`                                               |
| **Total fallos de página**                         | `MemoryMonitor.qml` → Estadísticas          | `MemoryMonitorViewModel.totalPageFaults`                         | `MemoryStats.totalPageFaults`                                                    |
| **Total reemplazos**                               | `MemoryMonitor.qml` → Estadísticas          | `MemoryMonitorViewModel.totalReplacements`                       | `MemoryStats.totalReplacements`                                                  |
| **Procesos bloqueados por E/S**                    | `BlockingPanel.qml` → ListView + Contadores | `BlockingEventsViewModel.blockedByIOCount`                       | `Simulator::getBlockedProcesses()`                                               |
| **Procesos bloqueados por Memoria**                | `BlockingPanel.qml` → ListView + Contadores | `BlockingEventsViewModel.blockedByMemoryCount`                   | `Simulator::getMemoryWaitQueue()`                                                |
| **Notificaciones Bloqueado→Listo**                 | `BlockingPanel.qml` → Log de eventos        | `BlockingEventsViewModel.eventList`                              | Signal `processStateChanged`                                                     |

---

**Última actualización**: 24 de noviembre de 2025  
**Versión**: 1.0  
**Contacto**: Issue #24 en GitHub
