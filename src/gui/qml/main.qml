import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1400
    height: 900
    title: "WaOS - Monitor del Sistema Operativo"
    
    Rectangle {
        anchors.fill: parent
        color: "#ECEFF1"
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10
            
            // Header
            Rectangle {
                Layout.fillWidth: true
                height: 60
                color: "#1976D2"
                radius: 5
                
                Text {
                    anchors.centerIn: parent
                    text: "WAOS - SIMULADOR DE SISTEMA OPERATIVO"
                    font.pixelSize: 24
                    font.bold: true
                    color: "white"
                }
            }
            
            // Control Panel
            ControlPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
            }
            
            // Main Content Area with Tabs
            TabBar {
                id: tabBar
                Layout.fillWidth: true
                
                TabButton {
                    text: "Planificación"
                }
                TabButton {
                    text: "Memoria"
                }
                TabButton {
                    text: "Log de Ejecución"
                }
            }
            
            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: tabBar.currentIndex
                
                // Panel de Planificación
                ProcessMonitor {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
                
                // Panel de Memoria
                MemoryMonitor {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
                
                // Log de Ejecución
                ExecutionLog {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
            
            // Panel de Bloqueos (Persistente)
            BlockingPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 200
            }
            
            // Footer
            Rectangle {
                Layout.fillWidth: true
                height: 30
                color: "#37474F"
                radius: 3
                
                Text {
                    anchors.centerIn: parent
                    text: "UNSA - Curso de Sistemas Operativos 2025"
                    font.pixelSize: 12
                    color: "white"
                }
            }
        }

        // --- Floating Control Bar ---
        Rectangle {
            id: floatingControls
            width: 600
            height: 70
            radius: 35
            color: "#ffffff"
            border.color: "#e0e0e0"
            border.width: 1
            
            // Shadow effect (simulated with a darker rectangle behind if DropShadow not available, 
            // but here we use simple z-ordering and border)
            z: 100 

            anchors.bottom: parent.bottom
            anchors.bottomMargin: 50
            anchors.horizontalCenter: parent.horizontalCenter

            // Shadow simulation
            Rectangle {
                anchors.fill: parent
                anchors.topMargin: 2
                anchors.leftMargin: 2
                z: -1
                radius: 35
                color: "#20000000"
            }

            RowLayout {
                anchors.centerIn: parent
                spacing: 20

                // Reset (Square Icon)
                RoundButton {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    onClicked: simulationController.reset()
                    ToolTip.visible: hovered
                    ToolTip.text: "Reset Simulation"
                    
                    contentItem: Rectangle {
                        anchors.centerIn: parent
                        width: 14
                        height: 14
                        color: "#555555"
                    }
                }

                // Step (Arrow + Bar Icon)
                RoundButton {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    enabled: !simulationController.isRunning
                    onClicked: simulationController.step()
                    ToolTip.visible: hovered
                    ToolTip.text: "Step Forward"

                    contentItem: Canvas {
                        anchors.fill: parent
                        anchors.margins: 10
                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.fillStyle = parent.enabled ? "#555555" : "#aaaaaa";
                            ctx.reset();
                            
                            // Triangle
                            ctx.beginPath();
                            ctx.moveTo(0, 0);
                            ctx.lineTo(12, 10);
                            ctx.lineTo(0, 20);
                            ctx.closePath();
                            ctx.fill();
                            
                            // Bar
                            ctx.beginPath();
                            ctx.rect(14, 0, 4, 20);
                            ctx.fill();
                        }
                    }
                }

                // Play/Pause (Big Button)
                RoundButton {
                    Layout.preferredWidth: 60
                    Layout.preferredHeight: 60
                    highlighted: true
                    onClicked: {
                        if (simulationController.isRunning) simulationController.stop()
                        else simulationController.start()
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: simulationController.isRunning ? "Pause" : "Start"
                    
                    contentItem: Item {
                        anchors.fill: parent
                        
                        // Play Icon (Triangle)
                        Canvas {
                            anchors.fill: parent
                            visible: !simulationController.isRunning
                            onPaint: {
                                var ctx = getContext("2d");
                                ctx.fillStyle = "#ffffff";
                                ctx.reset();
                                ctx.beginPath();
                                ctx.moveTo(22, 18);
                                ctx.lineTo(42, 30);
                                ctx.lineTo(22, 42);
                                ctx.closePath();
                                ctx.fill();
                            }
                        }
                        
                        // Pause Icon (Bars)
                        Row {
                            anchors.centerIn: parent
                            spacing: 6
                            visible: simulationController.isRunning
                            Rectangle { width: 6; height: 24; color: "#ffffff" }
                            Rectangle { width: 6; height: 24; color: "#ffffff" }
                        }
                    }
                }

                // Speed Control
                RowLayout {
                    spacing: 8
                    Label { text: "Slow"; font.pixelSize: 10; color: "#666" }
                    Slider {
                        from: 100
                        to: 2000
                        value: simulationController.tickInterval
                        onMoved: simulationController.tickInterval = value
                        Layout.preferredWidth: 120
                    }
                    Label { text: "Fast"; font.pixelSize: 10; color: "#666" }
                }
                
                Label { 
                    text: simulationController.tickInterval + " ms" 
                    font.family: "Monospace"
                    font.bold: true
                    color: "#555"
                }
            }
        }
    }
}
