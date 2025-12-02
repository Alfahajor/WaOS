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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        Label {
            text: "Memory Monitor"
            font.bold: true
            font.pixelSize: 20
            color: textColor
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

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20

            // Physical Memory Grid
            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 3
                color: "transparent"
                border.color: borderColor
                border.width: 1
                radius: 8
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    Label { text: "Physical Frames"; font.bold: true; color: textColor }
                    
                    GridView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        cellWidth: 70
                        cellHeight: 70
                        model: memoryViewModel.frameList

                        delegate: Rectangle {
                            width: 60
                            height: 60
                            color: model.modelData.color === "#ffffff" ? "#313244" : model.modelData.color // Map white to dark empty
                            border.color: borderColor
                            radius: 8
                            
                            ColumnLayout {
                                anchors.centerIn: parent
                                Label { text: "F" + model.modelData.frameId; font.pixelSize: 10; color: "#a6adc8" }
                                Label { 
                                    text: model.modelData.label
                                    font.pixelSize: 12
                                    font.bold: true
                                    color: "#11111b" // Text on colored block
                                    elide: Text.ElideRight
                                    Layout.maximumWidth: 55 
                                }
                            }
                        }
                    }
                }
            }

            // Page Table Panel
            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 2
                color: "transparent"
                border.color: borderColor
                border.width: 1
                radius: 8
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    
                    Label { text: "Page Table"; font.bold: true; color: textColor }
                    
                    RowLayout {
                        Label { text: "Process:"; color: textColor }
                        ComboBox {
                            model: memoryViewModel.processList
                            Layout.fillWidth: true
                            
                            delegate: ItemDelegate {
                                width: parent.width
                                contentItem: Text { text: modelData; color: textColor }
                                background: Rectangle { color: parent.highlighted ? accentColor : bgCard }
                            }
                            contentItem: Text { text: parent.displayText; color: textColor; verticalAlignment: Text.AlignVCenter; leftPadding: 10 }
                            background: Rectangle { color: bgCard; radius: 4; border.color: borderColor }

                            onActivated: (index) => { memoryViewModel.selectedPid = model[index] }
                            Component.onCompleted: { currentIndex = indexOfValue(memoryViewModel.selectedPid) }
                            onModelChanged: { currentIndex = indexOfValue(memoryViewModel.selectedPid) }
                        }
                    }

                    // Header
                    Rectangle {
                        Layout.fillWidth: true
                        height: 30
                        color: bgCard
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 5
                            Label { text: "Page"; Layout.preferredWidth: 50; font.bold: true; color: textColor }
                            Label { text: "Frame"; Layout.preferredWidth: 50; font.bold: true; color: textColor }
                            Label { text: "State"; Layout.fillWidth: true; font.bold: true; color: textColor }
                        }
                    }

                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: memoryViewModel.pageTable
                        
                        delegate: Rectangle {
                            width: parent.width
                            height: 30
                            color: index % 2 == 0 ? "#1e1e2e" : "#252535"
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 5
                                Label { text: model.modelData.pageNumber; Layout.preferredWidth: 50; color: textColor }
                                Label { text: model.modelData.frameNumber; Layout.preferredWidth: 50; color: textColor }
                                Label { 
                                    text: model.modelData.state
                                    Layout.fillWidth: true
                                    color: model.modelData.state === "Valid" ? "#a6e3a1" : "#f38ba8"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
