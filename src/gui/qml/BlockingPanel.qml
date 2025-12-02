import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#f5f5f5"
    border.color: "#ddd"
    border.width: 1

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 20

        // I/O Blocked List
        ColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.3
            
            Label { text: "Bloqueados por E/S"; font.bold: true; color: "#D32F2F" }
            
            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: blockingViewModel.ioBlockedList
                
                delegate: Rectangle {
                    width: parent.width
                    height: 30
                    color: "white"
                    border.color: "#eee"
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        Label { text: "PID " + model.modelData.pid; font.bold: true }
                        Label { text: model.modelData.info; color: "#666" }
                    }
                }
            }
        }

        // Memory Blocked List
        ColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.3
            
            Label { text: "Esperando Memoria"; font.bold: true; color: "#F57C00" }
            
            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: blockingViewModel.memoryBlockedList
                
                delegate: Rectangle {
                    width: parent.width
                    height: 30
                    color: "white"
                    border.color: "#eee"
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        Label { text: "PID " + model.modelData.pid; font.bold: true }
                        Label { text: model.modelData.info; color: "#666" }
                    }
                }
            }
        }

        // Notifications
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            
            Label { text: "Notificaciones de Desbloqueo"; font.bold: true; color: "#388E3C" }
            
            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: blockingViewModel.notifications
                
                delegate: Text {
                    text: modelData
                    font.pixelSize: 12
                    padding: 2
                }
            }
        }
    }
}
