import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    border.color: "#ccc"
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        Label {
            text: "Process Monitor"
            font.bold: true
            font.pixelSize: 16
        }

        RowLayout {
            Label { text: "Avg Wait: " + processViewModel.avgWaitTime.toFixed(2) + " ms" }
            Label { text: "Avg Turnaround: " + processViewModel.avgTurnaroundTime.toFixed(2) + " ms" }
            Label { text: "CPU Util: " + processViewModel.cpuUtilization.toFixed(1) + "%" }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: processViewModel.processList

            delegate: Rectangle {
                width: parent.width
                height: 40
                color: index % 2 == 0 ? "#f9f9f9" : "#ffffff"
                border.color: "#eee"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    
                    Label { text: "PID: " + model.modelData.pid; Layout.preferredWidth: 60 }
                    Label { text: model.modelData.state; Layout.preferredWidth: 100; color: model.modelData.state === "Ejecutando" ? "green" : "black" }
                    Label { text: "Pri: " + model.modelData.priority; Layout.preferredWidth: 50 }
                    Label { text: "Wait: " + model.modelData.waitTime; Layout.preferredWidth: 60 }
                    Label { text: "CPU: " + model.modelData.cpuTime; Layout.preferredWidth: 60 }
                    Label { text: "Burst: " + model.modelData.currentBurst; Layout.fillWidth: true }
                }
            }
        }
    }
}
