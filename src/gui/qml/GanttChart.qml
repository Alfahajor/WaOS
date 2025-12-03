import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    height: 100 // 80px content + padding
    color: "transparent"
    
    property color textColor: "#cdd6f4"
    property color rulerColor: "#a6adc8"
    property int pixelPerTick: 30 // Increased for better visibility of numbers

    // Catppuccin Mocha Palette
    property var processColors: [
        "#89b4fa", // Blue
        "#f38ba8", // Red
        "#a6e3a1", // Green
        "#fab387", // Peach
        "#cba6f7", // Mauve
        "#f9e2af", // Yellow
        "#94e2d5", // Teal
        "#f5c2e7", // Pink
        "#eba0ac", // Maroon
        "#89dceb", // Sky
        "#f2cdcd", // Flamingo
        "#f5e0dc"  // Rosewater
    ]

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        Label {
            text: "CPU History (Gantt Chart)"
            font.bold: true
            color: textColor
            font.pixelSize: 14
            Layout.leftMargin: 5
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentHeight: 70 
            
            ListView {
                id: ganttList
                orientation: ListView.Horizontal
                model: ganttViewModel
                spacing: 0
                clip: true

                delegate: Column {
                    property int startT: model.startTick
                    property int dur: model.duration

                    // Block
                    Rectangle {
                        width: dur * root.pixelPerTick
                        height: 40
                        color: model.color
                        radius: 4
                        border.color: (model.pid === "IDLE" || model.pid === "CS") ? "transparent" : Qt.darker(color, 1.2)
                        border.width: 1
                        
                        Column {
                            anchors.centerIn: parent
                            Text {
                                text: (model.pid === "IDLE" || model.pid === "CS") ? model.pid : "P" + model.pid
                                font.bold: true
                                color: (model.pid === "IDLE" || model.pid === "CS") ? "#a6adc8" : "#11111b"
                                font.pixelSize: 12
                                anchors.horizontalCenter: parent.horizontalCenter
                                visible: parent.parent.width > 20
                            }
                            Text {
                                text: model.duration + "u"
                                font.pixelSize: 10
                                font.bold: true
                                color: (model.pid === "IDLE" || model.pid === "CS") ? "#a6adc8" : "#11111b"
                                opacity: 0.7
                                anchors.horizontalCenter: parent.horizontalCenter
                                visible: parent.parent.width > 30
                            }
                        }

                        ToolTip {
                            visible: mouseArea.containsMouse
                            text: {
                                if (model.pid === "CS") return "Context Switch (Overhead)"
                                if (model.pid === "IDLE") return "System Idle"
                                return "Process: P" + model.pid + "\nDuration: " + model.duration + "u"
                            }
                        }
                        
                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                        }
                    }

                    // Ruler Segment (Ticks for every unit)
                    Row {
                        width: dur * root.pixelPerTick
                        height: 20
                        
                        Repeater {
                            model: dur
                            
                            Item {
                                width: root.pixelPerTick
                                height: parent.height
                                
                                // Tick Mark
                                Rectangle {
                                    width: 1
                                    height: 4
                                    color: root.rulerColor
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                }
                                
                                // Tick Label
                                Text {
                                    text: (startT + index)
                                    color: root.rulerColor
                                    font.pixelSize: 10
                                    font.family: "Consolas"
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                    anchors.topMargin: 6
                                    anchors.leftMargin: -width/2 + 1 // Center on tick
                                }
                            }
                        }
                    }
                }

                // Auto-scroll
                onCountChanged: {
                    Qt.callLater(function() {
                        positionViewAtEnd()
                    })
                }
            }
        }
    }
}
