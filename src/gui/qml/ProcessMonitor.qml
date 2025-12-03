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
    property color bgRowOdd: "#1e1e24"
    property color bgRowEven: "#121216"
    property color borderColor: "#2d3436"
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        RowLayout {
            Layout.fillWidth: true
            Label {
                text: "Process Monitor"
                font.bold: true
                font.pixelSize: 20
                color: textColor
                font.family: "Segoe UI"
            }
            
            // Algorithm Badge
            Rectangle {
                width: 100; height: 24
                radius: 12
                color: "transparent"
                border.color: accentColor
                border.width: 1
                
                Text {
                    anchors.centerIn: parent
                    text: "Round Robin" // Placeholder
                    color: accentColor
                    font.pixelSize: 12
                    font.bold: true
                }
            }
            
            Item { Layout.fillWidth: true }
        }

        // Metrics Cards
        RowLayout {
            spacing: 15
            
            Repeater {
                model: [
                    { label: "Avg Wait", value: processViewModel.avgWaitTime.toFixed(2) + " ms", icon: "⏱️" },
                    { label: "Avg Turnaround", value: processViewModel.avgTurnaroundTime.toFixed(2) + " ms", icon: "🔄" },
                    { label: "CPU Util", value: processViewModel.cpuUtilization.toFixed(1) + "%", icon: "💻" }
                ]
                
                Rectangle {
                    width: 180; height: 80
                    color: bgRowOdd
                    radius: 12
                    border.color: borderColor
                    border.width: 1
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 15
                        
                        // Icon Bubble
                        Rectangle {
                            width: 40; height: 40
                            radius: 20
                            color: Qt.lighter(bgRowOdd, 1.5)
                            Text {
                                anchors.centerIn: parent
                                text: modelData.icon
                                font.pixelSize: 20
                            }
                        }
                        
                        Column {
                            Layout.fillWidth: true
                            Text { text: modelData.label; color: "#b2bec3"; font.pixelSize: 12; font.bold: true }
                            Text { text: modelData.value; color: textColor; font.bold: true; font.pixelSize: 20 }
                        }
                    }
                }
            }
        }

        // Header
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: "transparent"
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 15
                anchors.rightMargin: 15
                spacing: 10
                
                Label { text: "PID"; Layout.preferredWidth: 60; font.bold: true; color: "#636e72" }
                Label { text: "STATE"; Layout.preferredWidth: 130; font.bold: true; color: "#636e72" }
                Label { text: "PRI"; Layout.preferredWidth: 50; font.bold: true; color: "#636e72" }
                Label { text: "WAIT"; Layout.preferredWidth: 60; font.bold: true; color: "#636e72" }
                Label { text: "CPU"; Layout.preferredWidth: 60; font.bold: true; color: "#636e72" }
                Label { text: "BURST PROGRESS"; Layout.fillWidth: true; font.bold: true; color: "#636e72" }
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: processViewModel.processList
            spacing: 8

            delegate: Rectangle {
                width: parent.width
                height: 50
                color: {
                    if (mainWindow.selectedPid === model.modelData.pid) {
                        return "#2d3436" // Highlight selected
                    }
                    return index % 2 == 0 ? bgRowEven : bgRowOdd
                }
                radius: 8
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        mainWindow.selectedPid = model.modelData.pid
                        mainWindow.selectionMode = "process"
                    }
                }

                // Selection Indicator Strip
                Rectangle {
                    width: 4
                    height: 30
                    anchors.left: parent.left
                    anchors.leftMargin: 4
                    anchors.verticalCenter: parent.verticalCenter
                    radius: 2
                    color: accentColor
                    visible: mainWindow.selectedPid === model.modelData.pid
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 15
                    anchors.rightMargin: 15
                    spacing: 10
                    
                    // PID
                    Label { 
                        text: "#" + model.modelData.pid
                        Layout.preferredWidth: 60
                        color: textColor
                        font.bold: true 
                        font.family: "Consolas"
                    }
                    
                    // State Badge
                    Rectangle {
                        Layout.preferredWidth: 130
                        height: 26
                        radius: 6
                        
                        property color stateColor: {
                             var s = model.modelData.state.toString();
                             if (s === "Ejecutando") return successColor;
                             if (s === "Listo") return warningColor;
                             if (s.startsWith("Bloqueado")) return errorColor;
                             return "#b2bec3";
                        }

                        color: Qt.rgba(stateColor.r, stateColor.g, stateColor.b, 0.15)
                        border.color: Qt.rgba(stateColor.r, stateColor.g, stateColor.b, 0.3)
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: model.modelData.state
                            color: parent.stateColor
                            font.bold: true
                            font.pixelSize: 11
                            font.capitalization: Font.AllUppercase
                            elide: Text.ElideRight
                            width: parent.width - 10
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }
                    
                    Label { text: model.modelData.priority; Layout.preferredWidth: 50; color: textColor; horizontalAlignment: Text.AlignHCenter }
                    Label { text: model.modelData.waitTime + "ms"; Layout.preferredWidth: 60; color: "#b2bec3"; font.family: "Consolas" }
                    Label { text: model.modelData.cpuTime + "ms"; Layout.preferredWidth: 60; color: "#b2bec3"; font.family: "Consolas" }
                    
                    // Burst Progress (Glow Dots)
                    Row {
                        Layout.fillWidth: true
                        height: 12
                        spacing: 6
                        
                        property int progress: (model.modelData.pid + model.modelData.cpuTime) % 10 + 1 
                        property bool isBlocked: model.modelData.state.toString().startsWith("Bloqueado")

                        Repeater {
                            model: 10
                            Rectangle {
                                width: 8; height: 8
                                radius: 4
                                color: {
                                    if (index < parent.progress) {
                                        if (parent.isBlocked) return errorColor
                                        return accentColor
                                    } else {
                                        return "#2d3436"
                                    }
                                }
                                
                                border.color: (index < parent.progress) ? Qt.lighter(color, 1.5) : "transparent"
                                border.width: (index < parent.progress) ? 1 : 0
                                
                                Rectangle {
                                    anchors.centerIn: parent
                                    width: 2; height: 2; radius: 1
                                    color: "#fff"
                                    visible: index < parent.progress
                                    opacity: 0.5
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
