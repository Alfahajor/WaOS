import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

Rectangle {
    id: controlPanel
    color: "#f5f5f5"
    border.color: "#dcdcdc"
    
    // Allow the height to adjust to content
    implicitHeight: mainCol.implicitHeight + 20
    Layout.fillWidth: true

    ColumnLayout {
        id: mainCol
        anchors.fill: parent
        anchors.margins: 10
        spacing: 15

        // --- Section 1: Configuration (Scheduler & Memory) ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            // Scheduler Configuration
            RowLayout {
                spacing: 8
                Label { text: "Scheduler:"; font.bold: true }
                ComboBox {
                    id: schedulerCombo
                    model: ["FCFS", "Round Robin", "SJF", "Priority"]
                    currentIndex: 0
                    Layout.preferredWidth: 140
                }
            }

            // Quantum (Only for Round Robin)
            RowLayout {
                visible: schedulerCombo.currentText === "Round Robin"
                spacing: 8
                Label { text: "Quantum:"; font.bold: true }
                SpinBox {
                    id: quantumSpin
                    from: 1
                    to: 100
                    value: 5
                    editable: true
                    Layout.preferredWidth: 100
                }
            }

            // Memory Configuration
            RowLayout {
                spacing: 8
                Label { text: "Memory:"; font.bold: true }
                ComboBox {
                    id: memoryCombo
                    model: ["FIFO", "LRU", "Optimal"]
                    currentIndex: 0
                    Layout.preferredWidth: 120
                }
            }
            
            Item { Layout.fillWidth: true } // Spacer
        }

        // --- Section 2: File Selection ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            enabled: !simulationController.isRunning

            Label { text: "Processes File:"; font.bold: true }
            
            TextField {
                id: pathField
                text: "tests/mock/test_processes.txt"
                Layout.fillWidth: true
                placeholderText: "Select a process file..."
                selectByMouse: true
                readOnly: false 
            }

            Button {
                text: "Browse..."
                onClicked: fileDialog.open()
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

                // --- Section 3: Simulation Controls ---
        // Removed redundant controls here as they are now in the floating bar
        Item { Layout.fillWidth: true; height: 10 } 
    }

    FileDialog {
    }

    FileDialog {
        id: fileDialog
        title: "Select Process File"
        nameFilters: ["Text files (*.txt)", "All files (*)"]
        currentFolder: StandardPaths.standardLocations(StandardPaths.DesktopLocation)[0]
        onAccepted: {
            var path = selectedFile.toString()
            // Clean up URL prefix for local files
            if (path.startsWith("file:///")) {
                path = path.substring(8)
            }
            pathField.text = path
        }
    }
}
