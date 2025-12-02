import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    border.color: "#ccc"
    clip: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        Label {
            text: "Log de Ejecución (CPU)"
            font.bold: true
            font.pixelSize: 16
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: executionLogViewModel
            clip: true
            
            delegate: Text {
                text: model.message
                font.family: "Consolas"
                font.pixelSize: 14
                padding: 2
            }

            // Auto-scroll to bottom
            onCountChanged: {
                positionViewAtEnd()
            }
        }
    }
}