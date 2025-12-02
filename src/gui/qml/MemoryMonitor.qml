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

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20

            // Physical Memory Grid
            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.6
                
                Label { text: "Memoria Física (Marcos)"; font.bold: true }
                
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

            // Page Table Panel
            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                
                Label { text: "Tabla de Páginas"; font.bold: true }
                
                RowLayout {
                    Label { text: "Proceso:" }
                    ComboBox {
                        model: memoryViewModel.processList
                        onActivated: (index) => {
                            memoryViewModel.selectedPid = model[index]
                        }
                        // Auto-select current
                        Component.onCompleted: {
                            currentIndex = indexOfValue(memoryViewModel.selectedPid)
                        }
                        onModelChanged: {
                             currentIndex = indexOfValue(memoryViewModel.selectedPid)
                        }
                    }
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: memoryViewModel.pageTable
                    
                    header: RowLayout {
                        width: parent.width
                        Label { text: "Página"; Layout.preferredWidth: 50; font.bold: true }
                        Label { text: "Marco"; Layout.preferredWidth: 50; font.bold: true }
                        Label { text: "Estado"; Layout.fillWidth: true; font.bold: true }
                    }

                    delegate: Rectangle {
                        width: parent.width
                        height: 30
                        color: index % 2 == 0 ? "#f0f0f0" : "white"
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 5
                            Label { text: model.modelData.pageNumber; Layout.preferredWidth: 50 }
                            Label { text: model.modelData.frameNumber; Layout.preferredWidth: 50 }
                            Label { text: model.modelData.state; Layout.fillWidth: true }
                        }
                    }
                }
            }
        }
    }
}
