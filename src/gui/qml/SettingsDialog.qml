import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

Dialog {
    id: settingsDialog
    title: "Configuration"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: 400

    // Theme Colors
    property color textColor: "#cdd6f4"
    property color bgInput: "#252535"
    property color borderColor: "#313244"
    property color accentColor: "#89b4fa"
    property color bgSurface: "#1e1e2e"

    background: Rectangle {
        color: bgSurface
        border.color: borderColor
        radius: 8
    }

    header: Label {
        text: settingsDialog.title
        color: textColor
        font.bold: true
        font.pixelSize: 18
        padding: 15
        background: Rectangle { color: "transparent" }
    }

    onAccepted: {
        simulationController.configure(
            schedulerCombo.currentText,
            quantumSpin.value,
            memoryCombo.currentText,
            framesSpin.value,
            pathField.text
        )
    }

    contentItem: ColumnLayout {
        spacing: 20
        
        // Scheduler
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8
            Label { text: "Scheduler Algorithm"; color: textColor; font.bold: true }
            ComboBox {
                id: schedulerCombo
                model: ["FCFS", "Round Robin", "SJF", "Priority"]
                currentIndex: 0
                Layout.fillWidth: true
                
                delegate: ItemDelegate {
                    width: schedulerCombo.width
                    contentItem: Text {
                        text: modelData
                        color: textColor
                        font: schedulerCombo.font
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: schedulerCombo.highlightedIndex === index ? accentColor : bgInput
                        opacity: schedulerCombo.highlightedIndex === index ? 0.3 : 1.0
                    }
                    highlighted: schedulerCombo.highlightedIndex === index
                }

                contentItem: Text {
                    leftPadding: 10
                    text: schedulerCombo.displayText
                    font: schedulerCombo.font
                    color: textColor
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    implicitHeight: 40
                    color: bgInput
                    border.color: borderColor
                    radius: 6
                }
            }
        }

        // Quantum (Only for Round Robin)
        ColumnLayout {
            visible: schedulerCombo.currentText === "Round Robin"
            Layout.fillWidth: true
            spacing: 8
            Label { text: "Time Quantum"; color: textColor; font.bold: true }
            SpinBox {
                id: quantumSpin
                from: 1
                to: 100
                value: 5
                editable: true
                Layout.fillWidth: true
                
                contentItem: TextInput {
                    z: 2
                    text: quantumSpin.textFromValue(quantumSpin.value, quantumSpin.locale)
                    font: quantumSpin.font
                    color: textColor
                    selectionColor: accentColor
                    selectedTextColor: "#ffffff"
                    horizontalAlignment: Qt.AlignLeft
                    verticalAlignment: Qt.AlignVCenter
                    readOnly: !quantumSpin.editable
                    validator: quantumSpin.validator
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                    leftPadding: 10
                }

                background: Rectangle {
                    implicitHeight: 40
                    color: bgInput
                    border.color: borderColor
                    radius: 6
                }
            }
        }

        // Memory
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8
            Label { text: "Memory Management"; color: textColor; font.bold: true }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                
                ComboBox {
                    id: memoryCombo
                    model: ["FIFO", "LRU", "Optimal"]
                    currentIndex: 0
                    Layout.fillWidth: true
                    
                    delegate: ItemDelegate {
                        width: memoryCombo.width
                        contentItem: Text {
                            text: modelData
                            color: textColor
                            font: memoryCombo.font
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: memoryCombo.highlightedIndex === index ? accentColor : bgInput
                            opacity: memoryCombo.highlightedIndex === index ? 0.3 : 1.0
                        }
                    }

                    contentItem: Text {
                        leftPadding: 10
                        text: memoryCombo.displayText
                        font: memoryCombo.font
                        color: textColor
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        implicitHeight: 40
                        color: bgInput
                        border.color: borderColor
                        radius: 6
                    }
                }

                Label { text: "Frames:"; color: textColor }
                
                SpinBox {
                    id: framesSpin
                    from: 4
                    to: 128
                    value: 16
                    editable: true
                    Layout.preferredWidth: 100
                    
                    contentItem: TextInput {
                        z: 2
                        text: framesSpin.textFromValue(framesSpin.value, framesSpin.locale)
                        font: framesSpin.font
                        color: textColor
                        selectionColor: accentColor
                        selectedTextColor: "#ffffff"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        readOnly: !framesSpin.editable
                        validator: framesSpin.validator
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }

                    background: Rectangle {
                        implicitHeight: 40
                        color: bgInput
                        border.color: borderColor
                        radius: 6
                    }
                }
            }
        }

        // File Selection
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8
            Label { text: "Process File"; color: textColor; font.bold: true }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                
                TextField {
                    id: pathField
                    text: "tests/mock/test_processes.txt"
                    Layout.fillWidth: true
                    placeholderText: "Select a process file..."
                    selectByMouse: true
                    color: textColor
                    selectionColor: accentColor
                    
                    background: Rectangle {
                        implicitHeight: 40
                        color: bgInput
                        border.color: borderColor
                        radius: 6
                    }
                }

                Button {
                    text: "Browse"
                    
                    background: Rectangle {
                        implicitWidth: 80
                        implicitHeight: 40
                        color: parent.down ? Qt.darker(bgInput, 1.2) : bgInput
                        border.color: borderColor
                        radius: 6
                    }
                    
                    contentItem: Text { 
                        text: "Browse"
                        color: textColor
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: fileDialog.open()
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select Process File"
        nameFilters: ["Text files (*.txt)", "All files (*)"]
        currentFolder: StandardPaths.standardLocations(StandardPaths.DesktopLocation)[0]
        onAccepted: {
            var path = selectedFile.toString()
            if (path.startsWith("file:///")) {
                path = path.substring(8)
            }
            pathField.text = path
        }
    }
}
