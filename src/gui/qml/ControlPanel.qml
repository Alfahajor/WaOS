import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#eee"
    border.color: "#ccc"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 20

        Button {
            text: simulationController.isRunning ? "Pause" : "Start"
            onClicked: {
                if (simulationController.isRunning) simulationController.stop()
                else simulationController.start()
            }
        }

        Button {
            text: "Step"
            enabled: !simulationController.isRunning
            onClicked: simulationController.step()
        }

        Button {
            text: "Reset"
            onClicked: simulationController.reset()
        }

        Label {
            text: "Speed:"
        }

        Slider {
            from: 100
            to: 2000
            value: simulationController.tickInterval
            onMoved: simulationController.tickInterval = value
        }
        
        Item { Layout.fillWidth: true } // Spacer
    }
}
