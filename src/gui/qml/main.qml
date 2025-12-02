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
    }
}
