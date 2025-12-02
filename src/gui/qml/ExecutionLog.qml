import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "transparent"
    clip: true

    // Theme Colors
    property color textColor: "#cdd6f4"
    property color logColor: "#a6e3a1" // Green terminal text
    property color bgConsole: "#11111b" // Darker console background

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        Label {
            text: "System Execution Log"
            font.bold: true
            font.pixelSize: 20
            color: textColor
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: bgConsole
            radius: 8
            border.color: "#313244"
            
            ListView {
                anchors.fill: parent
                anchors.margins: 10
                model: executionLogViewModel
                clip: true
                spacing: 2
                
                delegate: Text {
                    text: "> " + model.message
                    font.family: "Consolas"
                    font.pixelSize: 13
                    color: logColor
                    width: parent.width
                    wrapMode: Text.Wrap
                }

                // Auto-scroll to bottom
                onCountChanged: {
                    positionViewAtEnd()
                }
            }
        }
    }
}