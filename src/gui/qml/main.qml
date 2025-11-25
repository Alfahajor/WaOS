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
            
            // Control Panel (placeholder)
            Rectangle {
                Layout.fillWidth: true
                height: 120
                color: "white"
                border.color: "#90A4AE"
                border.width: 2
                radius: 5
                
                Text {
                    anchors.centerIn: parent
                    text: "Panel de Control (En desarrollo)"
                    font.pixelSize: 16
                    color: "#757575"
                }
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
                    text: "Bloqueos"
                }
            }
            
            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: tabBar.currentIndex
                
                // Panel de Planificación
                Rectangle {
                    color: "white"
                    border.color: "#BDBDBD"
                    border.width: 1
                    
                    Text {
                        anchors.centerIn: parent
                        text: "Panel de Planificación\n(Placeholder - Implementación en progreso)"
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 18
                        color: "#757575"
                    }
                }
                
                // Panel de Memoria
                Rectangle {
                    color: "white"
                    border.color: "#BDBDBD"
                    border.width: 1
                    
                    Text {
                        anchors.centerIn: parent
                        text: "Panel de Memoria\n(Placeholder - Implementación en progreso)"
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 18
                        color: "#757575"
                    }
                }
                
                // Panel de Bloqueos
                Rectangle {
                    color: "white"
                    border.color: "#BDBDBD"
                    border.width: 1
                    
                    Text {
                        anchors.centerIn: parent
                        text: "Panel de Bloqueos\n(Placeholder - Implementación en progreso)"
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 18
                        color: "#757575"
                    }
                }
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
