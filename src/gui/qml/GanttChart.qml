import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    border.color: "#ccc"
    clip: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        Label {
            text: "Diagrama de Gantt (Ejecución en CPU)"
            font.bold: true
            font.pixelSize: 16
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: ganttRow.width
            contentHeight: ganttRow.height

            Row {
                id: ganttRow
                spacing: 1
                
                Repeater {
                    model: ganttViewModel
                    
                    Rectangle {
                        width: model.duration * 20 // 20 pixels per tick
                        height: 50
                        color: model.color
                        border.color: "black"
                        border.width: 1
                        
                        Text {
                            anchors.centerIn: parent
                            text: "P" + model.pid
                            color: "white"
                            font.bold: true
                        }
                        
                        Text {
                            anchors.bottom: parent.bottom
                            anchors.left: parent.left
                            anchors.margins: 2
                            text: model.startTick
                            font.pixelSize: 10
                            color: "black"
                        }
                    }
                }
            }
        }
        
        Label {
            text: "Tiempo Total: " + ganttViewModel.totalTicks + " ticks"
        }
    }
}
