import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    border.color: "#ccc"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        Label {
            text: "Memory Monitor"
            font.bold: true
            font.pixelSize: 16
        }

        RowLayout {
            Label { text: "Faults: " + memoryViewModel.totalPageFaults }
            Label { text: "Replacements: " + memoryViewModel.totalReplacements }
            Label { text: "Hit Ratio: " + memoryViewModel.hitRatio.toFixed(1) + "%" }
        }

        GridView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            cellWidth: 60
            cellHeight: 60
            model: memoryViewModel.frameList

            delegate: Rectangle {
                width: 50
                height: 50
                color: model.modelData.color
                border.color: "black"
                radius: 5

                ColumnLayout {
                    anchors.centerIn: parent
                    Label { text: "F" + model.modelData.frameId; font.pixelSize: 10 }
                    Label { text: model.modelData.label; font.pixelSize: 10; elide: Text.ElideRight; Layout.maximumWidth: 45 }
                }
            }
        }
    }
}
