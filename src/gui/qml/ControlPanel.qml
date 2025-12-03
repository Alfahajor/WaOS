import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore

Rectangle {
    id: controlPanel
    color: "transparent" // Parent handles background
    
    // Allow the height to adjust to content
    implicitHeight: mainCol.implicitHeight + 20
    Layout.fillWidth: true

    // Theme Colors (Matching Main.qml)
    property color bgInput: "#252535"
    property color borderColor: "#313244"
    property color textColor: "#cdd6f4"
    property color accentColor: "#89b4fa"

    ColumnLayout {
        id: mainCol
        anchors.fill: parent
        anchors.margins: 0
        spacing: 15

        // --- Section 1: Configuration (Scheduler & Memory) ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            // Scheduler Configuration
            RowLayout {
                spacing: 10
                Label { text: "Scheduler"; color: controlPanel.textColor; font.bold: true }
                ComboBox {
                    id: schedulerCombo
                    model: ["FCFS", "Round Robin", "SJF", "Priority"]
                    currentIndex: 0
                    Layout.preferredWidth: 160
                    
                    delegate: ItemDelegate {
                        width: schedulerCombo.width
                        contentItem: Text {
                            text: modelData
                            color: schedulerCombo.highlightedIndex === index ? "#11111b" : controlPanel.textColor
                            font: schedulerCombo.font
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: schedulerCombo.highlightedIndex === index ? controlPanel.accentColor : controlPanel.bgInput
                            opacity: schedulerCombo.highlightedIndex === index ? 1.0 : 1.0
                        }
                        highlighted: schedulerCombo.highlightedIndex === index
                    }

                    indicator: Canvas {
                        x: schedulerCombo.width - width - schedulerCombo.rightPadding
                        y: schedulerCombo.topPadding + (schedulerCombo.availableHeight - height) / 2
                        width: 12
                        height: 8
                        contextType: "2d"

                        Connections {
                            target: schedulerCombo
                            function onPressedChanged() { schedulerCombo.indicator.requestPaint(); }
                        }

                        onPaint: {
                            context.reset();
                            context.moveTo(0, 0);
                            context.lineTo(width, 0);
                            context.lineTo(width / 2, height);
                            context.closePath();
                            context.fillStyle = schedulerCombo.pressed ? controlPanel.accentColor : controlPanel.textColor;
                            context.fill();
                        }
                    }

                    contentItem: Text {
                        leftPadding: 10
                        rightPadding: schedulerCombo.indicator.width + spacing
                        text: schedulerCombo.displayText
                        font: schedulerCombo.font
                        color: controlPanel.textColor
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }

                    background: Rectangle {
                        implicitWidth: 120
                        implicitHeight: 40
                        color: controlPanel.bgInput
                        border.color: controlPanel.borderColor
                        radius: 6
                    }
                }
            }

            // Quantum (Only for Round Robin)
            RowLayout {
                visible: schedulerCombo.currentText === "Round Robin"
                spacing: 10
                Label { text: "Quantum"; color: controlPanel.textColor; font.bold: true }
                SpinBox {
                    id: quantumSpin
                    from: 1
                    to: 100
                    value: 5
                    editable: true
                    Layout.preferredWidth: 120
                    
                    contentItem: TextInput {
                        z: 2
                        text: quantumSpin.textFromValue(quantumSpin.value, quantumSpin.locale)
                        font: quantumSpin.font
                        color: controlPanel.textColor
                        selectionColor: controlPanel.accentColor
                        selectedTextColor: "#ffffff"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        readOnly: !quantumSpin.editable
                        validator: quantumSpin.validator
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }

                    background: Rectangle {
                        implicitWidth: 140
                        color: controlPanel.bgInput
                        border.color: controlPanel.borderColor
                        radius: 6
                    }
                }
            }

            // Memory Configuration
            RowLayout {
                spacing: 10
                Label { text: "Memory"; color: controlPanel.textColor; font.bold: true }
                ComboBox {
                    id: memoryCombo
                    model: ["FIFO", "LRU", "Optimal"]
                    currentIndex: 0
                    Layout.preferredWidth: 140
                    
                    delegate: ItemDelegate {
                        width: memoryCombo.width
                        contentItem: Text {
                            text: modelData
                            color: memoryCombo.highlightedIndex === index ? "#11111b" : controlPanel.textColor
                            font: memoryCombo.font
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            color: memoryCombo.highlightedIndex === index ? controlPanel.accentColor : controlPanel.bgInput
                            opacity: memoryCombo.highlightedIndex === index ? 1.0 : 1.0
                        }
                    }

                    indicator: Canvas {
                        x: memoryCombo.width - width - memoryCombo.rightPadding
                        y: memoryCombo.topPadding + (memoryCombo.availableHeight - height) / 2
                        width: 12
                        height: 8
                        contextType: "2d"

                        Connections {
                            target: memoryCombo
                            function onPressedChanged() { memoryCombo.indicator.requestPaint(); }
                        }

                        onPaint: {
                            context.reset();
                            context.moveTo(0, 0);
                            context.lineTo(width, 0);
                            context.lineTo(width / 2, height);
                            context.closePath();
                            context.fillStyle = memoryCombo.pressed ? controlPanel.accentColor : controlPanel.textColor;
                            context.fill();
                        }
                    }

                    contentItem: Text {
                        leftPadding: 10
                        text: memoryCombo.displayText
                        font: memoryCombo.font
                        color: controlPanel.textColor
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        implicitWidth: 120
                        implicitHeight: 40
                        color: controlPanel.bgInput
                        border.color: controlPanel.borderColor
                        radius: 6
                    }
                }

                Label { text: "Frames"; color: controlPanel.textColor; font.bold: true }
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
                        color: controlPanel.textColor
                        selectionColor: controlPanel.accentColor
                        selectedTextColor: "#ffffff"
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter
                        readOnly: !framesSpin.editable
                        validator: framesSpin.validator
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }

                    background: Rectangle {
                        implicitWidth: 100
                        color: controlPanel.bgInput
                        border.color: controlPanel.borderColor
                        radius: 6
                    }
                }
            }
            
            Item { Layout.fillWidth: true } // Spacer
        }

        // --- Section 2: File Selection ---
        RowLayout {
            Layout.fillWidth: true
            spacing: 15
            enabled: !simulationController.isRunning

            Label { text: "Process File"; color: controlPanel.textColor; font.bold: true }
            
            TextField {
                id: pathField
                text: "tests/mock/test_processes.txt"
                Layout.fillWidth: true
                placeholderText: "Select a process file..."
                selectByMouse: true
                color: controlPanel.textColor
                selectionColor: controlPanel.accentColor
                
                background: Rectangle {
                    implicitHeight: 40
                    color: controlPanel.bgInput
                    border.color: controlPanel.borderColor
                    radius: 6
                }
            }

            Button {
                text: "Browse"
                icon.source: "qrc:/icons/folder.svg"
                icon.color: controlPanel.textColor
                
                background: Rectangle {
                    implicitWidth: 100
                    implicitHeight: 40
                    color: parent.down ? Qt.darker(controlPanel.bgInput, 1.2) : controlPanel.bgInput
                    border.color: controlPanel.borderColor
                    radius: 6
                }
                
                contentItem: RowLayout {
                    spacing: 5
                    Image { 
                        source: parent.icon.source
                        Layout.preferredWidth: 16; Layout.preferredHeight: 16
                        fillMode: Image.PreserveAspectFit
                        visible: true
                        // Tint the icon using a colored rectangle mask if needed, 
                        // but since we can't easily use ColorOverlay without import, 
                        // we rely on the user providing good SVGs or use a trick.
                        // Actually, let's just use Text + Icon property if possible, 
                        // but Button's default contentItem overrides icon display.
                        // Let's use a simple Text item and rely on the button background.
                    }
                    Text { 
                        text: "Browse"
                        color: controlPanel.textColor
                        font.bold: true
                    }
                }
                
                onClicked: fileDialog.open()
            }

            Button {
                text: "Load & Apply"
                icon.source: "qrc:/icons/check.svg"
                
                background: Rectangle {
                    implicitWidth: 140
                    implicitHeight: 40
                    color: parent.down ? Qt.darker(controlPanel.accentColor, 1.2) : controlPanel.accentColor
                    radius: 6
                }

                contentItem: RowLayout {
                    spacing: 5
                    Image { 
                        source: "qrc:/icons/check.svg"
                        Layout.preferredWidth: 16; Layout.preferredHeight: 16
                        fillMode: Image.PreserveAspectFit
                        visible: true
                        // Tinting would require ColorOverlay, assuming SVG is white or colored appropriately
                    }
                    Text { 
                        text: "Apply Config"
                        color: "#11111b" // Dark text on accent
                        font.bold: true
                        Layout.alignment: Qt.AlignCenter
                    }
                }

                onClicked: {
                    simulationController.configure(
                        schedulerCombo.currentText,
                        quantumSpin.value,
                        memoryCombo.currentText,
                        framesSpin.value,
                        pathField.text
                    )
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
            // Clean up URL prefix for local files
            if (path.startsWith("file:///")) {
                path = path.substring(8)
            }
            pathField.text = path
        }
    }
}
