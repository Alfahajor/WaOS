import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "transparent"
    
    // Theme Colors (Matching Main.qml)
    property color textColor: "#cdd6f4"
    property color accentColor: "#89b4fa"
    property color successColor: "#a6e3a1"
    property color bgRowOdd: "#252535"
    property color bgRowEven: "#1e1e2e"
    
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
            }
            Item { Layout.fillWidth: true }
        }

        // Metrics Cards
        RowLayout {
            spacing: 15
            
            Repeater {
                model: [
                    { label: "Avg Wait", value: processViewModel.avgWaitTime.toFixed(2) + " ms" },
                    { label: "Avg Turnaround", value: processViewModel.avgTurnaroundTime.toFixed(2) + " ms" },
                    { label: "CPU Util", value: processViewModel.cpuUtilization.toFixed(1) + "%" }
                ]
                
                Rectangle {
                    width: 160; height: 70
                    color: bgRowOdd
                    radius: 8
                    border.color: "#313244"
                    
                    Column {
                        anchors.centerIn: parent
                        Text { text: modelData.label; color: "#a6adc8"; font.pixelSize: 12 }
                        Text { text: modelData.value; color: accentColor; font.bold: true; font.pixelSize: 18 }
                    }
                }
            }
        }

        // Header
        Rectangle {
            Layout.fillWidth: true
            height: 40
            color: "#313244"
            radius: 5
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                spacing: 10
                
                Label { text: "PID"; Layout.preferredWidth: 60; font.bold: true; color: textColor }
                Label { text: "State"; Layout.preferredWidth: 100; font.bold: true; color: textColor }
                Label { text: "Pri"; Layout.preferredWidth: 50; font.bold: true; color: textColor }
                Label { text: "Wait"; Layout.preferredWidth: 60; font.bold: true; color: textColor }
                Label { text: "CPU"; Layout.preferredWidth: 60; font.bold: true; color: textColor }
                Label { text: "Burst Progress"; Layout.fillWidth: true; font.bold: true; color: textColor }
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: processViewModel.processList
            spacing: 5

            delegate: Rectangle {
                width: parent.width
                height: 45
                color: index % 2 == 0 ? bgRowEven : bgRowOdd
                radius: 5
                border.width: 0

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 10
                    
                    Label { text: model.modelData.pid; Layout.preferredWidth: 60; color: textColor; font.bold: true }
                    
                    Rectangle {
                        Layout.preferredWidth: 100
                        height: 24
                        radius: 4
                        color: {
                            switch(model.modelData.state) {
                                case "Ejecutando": return successColor;
                                case "Listo": return accentColor;
                                case "Bloqueado": return "#f38ba8"; // Red
                                default: return "#585b70";
                            }
                        }
                        
                        Text {
                            anchors.centerIn: parent
                            text: model.modelData.state
                            color: "#11111b"
                            font.bold: true
                            font.pixelSize: 11
                        }
                    }
                    
                    Label { text: model.modelData.priority; Layout.preferredWidth: 50; color: textColor }
                    Label { text: model.modelData.waitTime; Layout.preferredWidth: 60; color: textColor }
                    Label { text: model.modelData.cpuTime; Layout.preferredWidth: 60; color: textColor }
                    
                    // Burst Progress Bar
                    Rectangle {
                        Layout.fillWidth: true
                        height: 8
                        radius: 4
                        color: "#313244"
                        
                        Rectangle {
                            width: parent.width * 0.5 // Placeholder logic, ideally model has progress
                            height: parent.height
                            radius: 4
                            color: accentColor
                        }
                        
                        Label {
                            anchors.centerIn: parent
                            text: model.modelData.currentBurst
                            font.pixelSize: 10
                            color: textColor
                        }
                    }
                }
            }
        }
    }
}
