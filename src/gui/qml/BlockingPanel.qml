import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "transparent" // Transparent to blend with main background
    
    // Theme Colors
    property color textColor: "#cdd6f4"
    property color bgCard: "#252535"
    property color borderColor: "#313244"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 20

        // I/O Blocked List
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.3
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
            Layout.preferredWidth: parent.width * 0.3
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

        // Notifications
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: bgCard
            radius: 8
            border.color: borderColor
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                Label { text: "Unlock Notifications"; font.bold: true; color: "#a6e3a1" } // Green
                
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: blockingViewModel.notifications
                    spacing: 2
                    
                    delegate: Text {
                        text: "> " + modelData
                        font.pixelSize: 12
                        font.family: "Consolas"
                        color: "#a6e3a1"
                        padding: 2
                    }
                }
            }
        }
    }
}
