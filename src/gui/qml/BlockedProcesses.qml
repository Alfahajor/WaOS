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
                Flickable {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    contentWidth: readyRow.width
                    contentHeight: height
                    
                    Row {
                        id: readyRow
                        spacing: 5
                        height: parent.height
                        
                        Repeater {
                            model: processViewModel.processList
                            
                            delegate: Item {
                                visible: model.modelData.state === "Listo"
                                width: visible ? 50 : 0
                                height: parent.height
                                
                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: 2
                                    color: "#2a2a35"
                                    radius: 4
                                    border.color: "#f9e2af" // Yellow/Gold
                                    border.width: 1
                                    visible: parent.visible
                                    
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
                        width: 50
                        height: parent.height - 4
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#2a2a35"
                        radius: 4
                        border.color: "#f38ba8" // Red
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: "P" + model.modelData.pid
                            color: "#f38ba8" // Red Text
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
                        width: 50
                        height: parent.height - 4
                        anchors.verticalCenter: parent.verticalCenter
                        color: "#2a2a35"
                        radius: 4
                        border.color: "#fab387" // Orange
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
