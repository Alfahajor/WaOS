import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "transparent"
    
    // Theme Colors (WaOS Theme)
    property color textColor: "#dfe6e9"
    property color accentColor: "#6c5ce7" // Violet
    property color successColor: "#00b894" // Green
    property color errorColor: "#d63031"   // Red
    property color warningColor: "#fdcb6e" // Orange
    property color bgRowOdd: "transparent" // No alternating background
    property color bgRowEven: "transparent"
    property color borderColor: "#2d3436"
    property color bgCard: "#1e1e24" // Lighter for metrics
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "PROCESS CONTROL BLOCK"
                font.bold: true
                font.pixelSize: 14
                color: "#636e72"
                font.family: "Segoe UI"
            }
            
            Item { Layout.fillWidth: true }

            // Algorithm Badge (Dynamic Strategy)
            Rectangle {
                width: 80; height: 24
                radius: 4
                color: Qt.rgba(108/255, 92/255, 231/255, 0.2)
                border.color: accentColor
                border.width: 1
                
                Text {
                    anchors.centerIn: parent
                    text: "RR (Q=2)" // Mock dynamic strategy
                    color: accentColor
                    font.pixelSize: 11
                    font.bold: true
                }
            }
        }

        // Metrics Cards
        RowLayout {
            spacing: 15
            
            Repeater {
                model: [
                    { label: "AVG WAIT TIME", value: processViewModel.avgWaitTime.toFixed(1) + " ms" },
                    { label: "AVG TURNAROUND", value: processViewModel.avgTurnaroundTime.toFixed(1) + " ms" },
                    { label: "THROUGHPUT", value: "0.4 p/ms" }, // Mock
                    { label: "ACTIVE PROCS", value: processViewModel.processList.length }
                ]
                
                Rectangle {
                    Layout.fillWidth: true
                    height: 60
                    color: bgCard // Lighter background
                    radius: 4
                    border.color: borderColor
                    border.width: 1
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 2
                        
                        Text { 
                            text: modelData.label
                            color: "#636e72"
                            font.pixelSize: 10
                            font.bold: true
                        }
                        Text { 
                            text: modelData.value
                            color: textColor
                            font.bold: true
                            font.pixelSize: 16
                        }
                    }
                }
            }
        }

        // Header
        Rectangle {
            Layout.fillWidth: true
            height: 30
            color: "transparent"
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 0 // Using Layout.preferredWidth for distribution
                
                Label { text: "PID"; Layout.preferredWidth: 60; font.bold: true; color: textColor }
                Label { text: "State"; Layout.preferredWidth: 120; font.bold: true; color: textColor; horizontalAlignment: Text.AlignHCenter }
                Label { text: "Pri"; Layout.preferredWidth: 60; font.bold: true; color: textColor; horizontalAlignment: Text.AlignHCenter }
                Label { text: "Arr"; Layout.preferredWidth: 60; font.bold: true; color: textColor; horizontalAlignment: Text.AlignHCenter } // Added Arrival
                Label { text: "Wait"; Layout.preferredWidth: 60; font.bold: true; color: textColor; horizontalAlignment: Text.AlignHCenter }
                Label { text: "Burst (Dots)"; Layout.fillWidth: true; font.bold: true; color: textColor }
            }
        }

        // Divider
        Rectangle { Layout.fillWidth: true; height: 1; color: borderColor }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: processViewModel.processList
            spacing: 0 // No spacing, just dividers

            delegate: Item {
                width: parent.width
                height: 50
                
                // Selection Background (Subtle)
                Rectangle {
                    anchors.fill: parent
                    color: (mainWindow.selectedPid === model.modelData.pid) ? Qt.rgba(108/255, 92/255, 231/255, 0.05) : "transparent"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        mainWindow.selectedPid = model.modelData.pid
                        mainWindow.selectionMode = "process"
                    }
                }

                // Selection Indicator Strip
                Rectangle {
                    width: 2
                    height: parent.height
                    anchors.left: parent.left
                    color: accentColor
                    visible: mainWindow.selectedPid === model.modelData.pid
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 0
                    
                    // PID
                    Label { 
                        text: "P" + model.modelData.pid
                        Layout.preferredWidth: 60
                        color: textColor
                        font.pixelSize: 14
                    }
                    
                    // State Badge (Centered)
                    Item {
                        Layout.preferredWidth: 120
                        Layout.fillHeight: true
                        
                        Rectangle {
                            anchors.centerIn: parent
                            // width moved to bottom
                            height: 22
                            radius: 4
                            
                            property color stateColor: {
                                 var s = model.modelData.state.toString();
                                 if (s === "Ejecutando") return successColor;
                                 if (s === "Listo") return warningColor;
                                 if (s.startsWith("Bloqueado")) return errorColor;
                                 return "#b2bec3";
                            }
                            
                            // Solid background for badge as per image reference (dark text on colored bg)
                            // OR colored text on dark bg. Reference shows: Green BG with Dark Text for Running?
                            // Actually reference shows: Green Text on Dark Green BG.
                            color: Qt.rgba(stateColor.r, stateColor.g, stateColor.b, 0.15)
                            
                            Text {
                                id: stateText
                                anchors.centerIn: parent
                                text: {
                                    var s = model.modelData.state.toString();
                                    if (s === "Ejecutando") return "RUNNING";
                                    if (s === "Listo") return "READY";
                                    if (s.startsWith("Bloqueado")) return "BLOCKED";
                                    return s.toUpperCase();
                                }
                                color: parent.stateColor
                                font.bold: true
                                font.pixelSize: 11
                            }
                            
                            width: stateText.implicitWidth + 16 // Adaptive width defined after Text
                        }
                    }
                    
                    Label { text: model.modelData.priority; Layout.preferredWidth: 60; color: textColor; horizontalAlignment: Text.AlignHCenter }
                    Label { text: "0"; Layout.preferredWidth: 60; color: textColor; horizontalAlignment: Text.AlignHCenter } // Mock Arrival
                    Label { text: model.modelData.waitTime; Layout.preferredWidth: 60; color: textColor; horizontalAlignment: Text.AlignHCenter }
                    
                    // Burst Progress (Dots + Text)
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        // Dots
                        Row {
                            spacing: 4
                            property int progress: (model.modelData.pid + model.modelData.cpuTime) % 10 + 1 
                            property bool isBlocked: model.modelData.state.toString().startsWith("Bloqueado")

                            Repeater {
                                model: 10
                                Rectangle {
                                    width: 6; height: 6
                                    radius: 3
                                    color: {
                                        if (index < parent.progress) {
                                            if (parent.isBlocked) return errorColor
                                            return accentColor // Blue/Violet
                                        } else {
                                            return "#333" // Dark Gray
                                        }
                                    }
                                }
                            }
                        }
                        
                        // Text Indicator
                        Text {
                            text: {
                                var s = model.modelData.state.toString();
                                if (s.startsWith("Bloqueado")) return "Wait";
                                return (10 - ((model.modelData.pid + model.modelData.cpuTime) % 10)) + "rem";
                            }
                            color: {
                                var s = model.modelData.state.toString();
                                if (s.startsWith("Bloqueado")) return errorColor;
                                return "#636e72";
                            }
                            font.pixelSize: 11
                        }
                    }
                }
                
                // Bottom Divider
                Rectangle {
                    width: parent.width
                    height: 1
                    anchors.bottom: parent.bottom
                    color: borderColor
                    opacity: 0.5
                }
            }
        }
    }
}
