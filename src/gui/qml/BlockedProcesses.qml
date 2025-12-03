import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    // Theme Colors (WaOS Theme)
    property color textColor: "#dfe6e9"
    property color bgDark: "#121216"
    property color bgTrack: "#252530" // Lighter background for tracks (Visible)
    
    // Specific Queue Colors
    property color readyColor: "#f1c40f"   // Yellow/Gold
    property color blockedColor: "#e74c3c" // Red
    property color memoryColor: "#e67e22"  // Orange
    
    property color borderColor: "#2d3436"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8 // Reduced spacing

        // Divider at the top
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: borderColor
            opacity: 0.5
        }

        // 1. Ready Queue
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 32 // Reduced height
            spacing: 10
            
            Label {
                Layout.preferredWidth: 80
                text: "Ready Q."
                font.bold: true
                font.pixelSize: 13 // Larger font
                color: "white" // User requested white for Ready Q section
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
            }
            
            // Queue Container (Track)
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: bgTrack // Visible lighter background
                border.color: borderColor
                border.width: 1
                radius: 4
                
                ListView {
                    anchors.fill: parent
                    anchors.margins: 4
                    orientation: ListView.Horizontal
                    spacing: 4 // Tighter spacing
                    clip: true
                    model: processViewModel.processList
                    
                    delegate: Item {
                        visible: model.modelData.state === "Listo"
                        width: visible ? height : 0 // Square aspect ratio (width = height)
                        height: parent.height
                        
                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            border.color: readyColor
                            border.width: 1
                            radius: 3
                            visible: parent.visible
                            
                            Text {
                                anchors.centerIn: parent
                                text: "P" + model.modelData.pid
                                color: "white" // White text inside box
                                font.bold: true
                                font.pixelSize: 11
                            }
                        }
                    }
                }
            }
        }

        // 2. Blocked I/O Queue
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            spacing: 10
            
            Label {
                Layout.preferredWidth: 80
                text: "Blocked I/O"
                font.bold: true
                font.pixelSize: 13
                color: blockedColor
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
            }
            
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: bgTrack
                border.color: borderColor
                border.width: 1
                radius: 4
                
                ListView {
                    anchors.fill: parent
                    anchors.margins: 4
                    orientation: ListView.Horizontal
                    spacing: 4
                    clip: true
                    model: blockingViewModel.ioBlockedList
                    
                    delegate: Item {
                        width: height // Square
                        height: parent.height
                        
                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            border.color: blockedColor
                            border.width: 1
                            radius: 3
                            
                            Text {
                                anchors.centerIn: parent
                                text: "P" + model.modelData.pid
                                color: "white"
                                font.bold: true
                                font.pixelSize: 11
                            }
                        }
                    }
                }
            }
        }

        // 3. Wait Mem Queue
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 32
            spacing: 10
            
            Label {
                Layout.preferredWidth: 80
                text: "Wait Mem"
                font.bold: true
                font.pixelSize: 13
                color: memoryColor
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
            }
            
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: bgTrack
                border.color: borderColor
                border.width: 1
                radius: 4
                
                ListView {
                    anchors.fill: parent
                    anchors.margins: 4
                    orientation: ListView.Horizontal
                    spacing: 4
                    clip: true
                    model: blockingViewModel.memoryBlockedList
                    
                    delegate: Item {
                        width: height // Square
                        height: parent.height
                        
                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            border.color: memoryColor
                            border.width: 1
                            radius: 3
                            
                            Text {
                                anchors.centerIn: parent
                                text: "P" + model.modelData.pid
                                color: "white"
                                font.bold: true
                                font.pixelSize: 11
                            }
                        }
                    }
                }
            }
        }
        
        Item { Layout.fillHeight: true } // Spacer
    }
}



