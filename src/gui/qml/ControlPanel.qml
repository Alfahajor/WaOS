import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Rectangle {
    color: "#eee"
    border.color: "#ccc"
    height: 140 // Increased height for config rows

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // --- Configuration Row ---
        RowLayout {
            enabled: !simulationController.isRunning
            spacing: 10

            Label { text: "Scheduler:" }
            ComboBox {
                id: schedulerCombo
                model: ["FCFS", "Round Robin", "SJF", "Priority"]
                currentIndex: 0
            }

            Label { 
                text: "Quantum:" 
                visible: schedulerCombo.currentText === "Round Robin"
            }
            SpinBox {
                id: quantumSpin
                from: 1
                to: 100
                value: 5
                visible: schedulerCombo.currentText === "Round Robin"
            }

            Label { text: "Memory:" }
            ComboBox {
                id: memoryCombo
                model: ["FIFO", "LRU", "Optimal"]
                currentIndex: 0
            }
        }

        // --- File & Load Row ---
        RowLayout {
            enabled: !simulationController.isRunning
            spacing: 10

            Label { text: "Processes File:" }
            TextField {
                id: pathField
                text: "tests/mock/test_processes.txt"
                Layout.fillWidth: true
                placeholderText: "Path to process file..."
            }

            Button {
                text: "Load & Apply"
                highlighted: true
                onClicked: {
                    simulationController.configure(
                        schedulerCombo.currentText,
                        quantumSpin.value,
                        memoryCombo.currentText,
                        pathField.text
                    )
                }
            }
        }

        // --- Control Row ---
        RowLayout {
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
            
            ColumnLayout {
                Label { text: "Current: " + simulationController.schedulerAlgorithm; font.pixelSize: 10 }
                Label { text: "Current: " + simulationController.memoryAlgorithm; font.pixelSize: 10 }
            }
        }
    }
}
