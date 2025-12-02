import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "transparent"

    // Theme Colors
    property color textColor: "#cdd6f4"
    property color accentColor: "#cba6f7" // Purple for Memory
    property color bgCard: "#252535"
    property color borderColor: "#313244"
    property color bgFrame: "#2a2a35"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // Header
        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Memory Grid"
                font.bold: true
                font.pixelSize: 20
                color: textColor
            }
            Item { Layout.fillWidth: true }
        }

        // Metrics
        RowLayout {
            spacing: 15
            Repeater {
                model: [
                    { label: "Page Faults", value: memoryViewModel.totalPageFaults },
                    { label: "Replacements", value: memoryViewModel.totalReplacements },
                    { label: "Hit Ratio", value: memoryViewModel.hitRatio.toFixed(1) + "%" }
                ]
                Rectangle {
                    width: 140; height: 60
                    color: bgCard
                    radius: 8
                    border.color: borderColor
                    Column {
                        anchors.centerIn: parent
                        Text { text: modelData.label; color: "#a6adc8"; font.pixelSize: 11 }
                        Text { text: modelData.value; color: accentColor; font.bold: true; font.pixelSize: 16 }
                    }
                }
            }
        }

        // Memory Grid
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            border.color: borderColor
            border.width: 1
            radius: 8
            
            GridView {
                anchors.fill: parent
                anchors.margins: 10
                clip: true
                cellWidth: 60
                cellHeight: 60
                model: memoryViewModel.frameList

                delegate: Rectangle {
                    width: 50
                    height: 50
                    color: bgFrame
                    radius: 4
                    
                    // Border Color Logic: Cyan (P1), Purple (P2), etc.
                    // Mock logic based on PID for color variation
                    property int pid: model.modelData.pid
                    property bool isFree: pid === -1
                    
                    border.color: {
                        if (isFree) return "#45475a" // Gray
                        if (pid % 2 === 0) return "#cba6f7" // Purple
                        return "#89dceb" // Cyan
                    }
                    border.width: isFree ? 1 : 2
                    opacity: isFree ? 0.5 : 1.0

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            // Update Global Selection
                            mainWindow.selectedFrame = model.modelData
                            mainWindow.selectionMode = "frame"
                        }
                    }

                    // Frame Number (Corner)
                    Text {
                        text: "F" + model.modelData.frameId
                        font.pixelSize: 10
                        color: "#a6adc8"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.margins: 4
                    }

                    // PID (Center)
                    Text {
                        text: parent.isFree ? "-" : "P" + parent.pid
                        font.pixelSize: 16
                        font.bold: true
                        color: textColor
                        anchors.centerIn: parent
                    }
                }
            }
        }
    }
}
