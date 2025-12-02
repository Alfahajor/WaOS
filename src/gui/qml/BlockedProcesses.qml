import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    // Theme Colors (inherited or set)
    property color textColor: "#cdd6f4"
    property color bgCard: "#252535"
    property color borderColor: "#313244"

    RowLayout {
        anchors.fill: parent
        spacing: 10

        // I/O Blocked List
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: bgCard
            radius: 8
            border.color: borderColor
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                Label { text: "Blocked (I/O)"; font.bold: true; color: "#f38ba8" } // Red
                
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: blockingViewModel.ioBlockedList
                    spacing: 5
                    
                    delegate: Rectangle {
                        width: parent.width
                        height: 35
                        color: "#1e1e2e"
                        radius: 4
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            Label { text: "PID " + model.modelData.pid; font.bold: true; color: textColor }
                            Label { text: model.modelData.info; color: "#a6adc8" }
                        }
                    }
                }
            }
        }

        // Memory Blocked List
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: bgCard
            radius: 8
            border.color: borderColor
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                Label { text: "Waiting for Memory"; font.bold: true; color: "#fab387" } // Orange
                
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: blockingViewModel.memoryBlockedList
                    spacing: 5
                    
                    delegate: Rectangle {
                        width: parent.width
                        height: 35
                        color: "#1e1e2e"
                        radius: 4
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            Label { text: "PID " + model.modelData.pid; font.bold: true; color: textColor }
                            Label { text: model.modelData.info; color: "#a6adc8" }
                        }
                    }
                }
            }
        }
    }
}
