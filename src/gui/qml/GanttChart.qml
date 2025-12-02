import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    height: 100 // 80px content + padding
    color: "transparent"
    
    property color textColor: "#cdd6f4"
    property color rulerColor: "#a6adc8"
    property int pixelPerTick: 20

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
            contentHeight: 60 // Height for blocks + ruler
            
            ListView {
                id: ganttList
                orientation: ListView.Horizontal
                model: ganttViewModel
                spacing: 0
                clip: true

                delegate: Column {
                    // Block
                    Rectangle {
                        width: model.duration * root.pixelPerTick
                        height: 40
                        color: model.color || "#89b4fa" // Fallback
                        radius: 4
                        border.color: Qt.darker(color, 1.2)
                        
                        Column {
                            anchors.centerIn: parent
                            Text {
                                text: "P" + model.pid
                                font.bold: true
                                color: "#11111b"
                                font.pixelSize: 12
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: model.duration + "u"
                                font.pixelSize: 10
                                color: "#11111b"
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }

                    // Ruler Segment
                    Item {
                        width: model.duration * root.pixelPerTick
                        height: 20
                        
                        // Start Tick Mark
                        Rectangle {
                            width: 1
                            height: 5
                            color: root.rulerColor
                            anchors.left: parent.left
                            anchors.top: parent.top
                        }
                        
                        // Start Tick Label
                        Text {
                            text: model.startTick
                            color: root.rulerColor
                            font.pixelSize: 10
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.topMargin: 6
                            anchors.leftMargin: -width/2 // Center on line
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
