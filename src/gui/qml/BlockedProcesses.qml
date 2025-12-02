import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    // Theme Colors
    property color textColor: "#cdd6f4"
    property color bgCard: "#252535"
    property color borderColor: "#313244"
    property color bgSwimlane: "#1e1e2e"

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        // 1. Ready Queue (Swimlane)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: bgCard
            radius: 8
            border.color: borderColor
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 10
                
                // Label
                Rectangle {
                    Layout.fillHeight: true
                    width: 80
                    color: "transparent"
                    Label { 
                        anchors.centerIn: parent
                        text: "Ready"
                        font.bold: true
                        color: "#89b4fa" // Blue
                    }
                }
                
                // Horizontal List
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    orientation: ListView.Horizontal
                    clip: true
                    spacing: 5
                    
                    // Filter processList for "Listo" state
                    model: processViewModel.processList
                    
                    delegate: Item {
                        visible: model.modelData.state === "Listo"
                        width: visible ? 40 : 0
                        height: parent.height
                        
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            color: bgSwimlane
                            radius: 4
                            border.color: "#89b4fa"
                            border.width: 1
                            
                            Text {
                                anchors.centerIn: parent
                                text: "P" + model.modelData.pid
                                color: textColor
                                font.bold: true
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }
        }

        // 2. Blocked (I/O) Queue (Swimlane)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: bgCard
            radius: 8
            border.color: borderColor
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 10
                
                Rectangle {
                    Layout.fillHeight: true
                    width: 80
                    color: "transparent"
                    Label { 
                        anchors.centerIn: parent
                        text: "Blocked"
                        font.bold: true
                        color: "#f38ba8" // Red
                    }
                }
                
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    orientation: ListView.Horizontal
                    clip: true
                    spacing: 5
                    model: blockingViewModel.ioBlockedList
                    
                    delegate: Rectangle {
                        width: 40
                        height: parent.height - 4
                        anchors.verticalCenter: parent.verticalCenter
                        color: bgSwimlane
                        radius: 4
                        border.color: "#f38ba8"
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: "P" + model.modelData.pid
                            color: textColor
                            font.bold: true
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }

        // 3. Waiting for Memory Queue (Swimlane)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: bgCard
            radius: 8
            border.color: borderColor
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 10
                
                Rectangle {
                    Layout.fillHeight: true
                    width: 80
                    color: "transparent"
                    Label { 
                        anchors.centerIn: parent
                        text: "Memory"
                        font.bold: true
                        color: "#fab387" // Orange
                    }
                }
                
                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    orientation: ListView.Horizontal
                    clip: true
                    spacing: 5
                    model: blockingViewModel.memoryBlockedList
                    
                    delegate: Rectangle {
                        width: 40
                        height: parent.height - 4
                        anchors.verticalCenter: parent.verticalCenter
                        color: bgSwimlane
                        radius: 4
                        border.color: "#fab387"
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: "P" + model.modelData.pid
                            color: textColor
                            font.bold: true
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }
        
        Item { Layout.fillHeight: true } // Spacer
    }
}
