import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "transparent"
    clip: true

    // Theme Colors
    property color textColor: "#ffffff" // Brighter white
    property color logColor: "#a6e3a1" 
    property color bgConsole: "transparent" // Background handled by parent container
    property color accentColor: "#89b4fa"
    property color errorColor: "#f38ba8"
    property color warnColor: "#fab387"
    property color successColor: "#a6e3a1"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Log List
        ListView {
            id: logListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: executionLogViewModel
            clip: true
            spacing: 0 
            
            delegate: ColumnLayout {
                width: ListView.view.width
                spacing: 4

                Text {
                    Layout.fillWidth: true
                    Layout.topMargin: 2
                    Layout.bottomMargin: 2
                    Layout.leftMargin: 5
                    
                    text: model.message
                    textFormat: Text.RichText
                    color: "#cdd6f4"
                    font.family: "Consolas"
                    font.pixelSize: 13
                    wrapMode: Text.Wrap
                }

                // Separator
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#313244" 
                    opacity: 0.5
                }
            }

            // Auto-scroll to bottom
            onCountChanged: {
                positionViewAtEnd()
            }
        }
    }
}