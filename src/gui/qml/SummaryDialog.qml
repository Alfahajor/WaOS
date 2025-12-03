import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    title: "Simulation Summary"
    modal: true
    standardButtons: Dialog.Close
    
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: 400
    
    background: Rectangle {
        color: "#1e1e2e"
        border.color: "#313244"
        radius: 8
    }
    
    header: Rectangle {
        color: "transparent"
        height: 40
        Text {
            anchors.centerIn: parent
            text: root.title
            color: "#cdd6f4"
            font.bold: true
            font.pixelSize: 16
        }
    }
    
    contentItem: ColumnLayout {
        spacing: 20
        
        // CPU Stats
        GroupBox {
            title: "CPU Statistics"
            Layout.fillWidth: true
            background: Rectangle { color: "transparent"; border.color: "#313244"; radius: 4 }
            label: Text { text: parent.title; color: "#89b4fa"; font.bold: true }
            
            GridLayout {
                columns: 2
                rowSpacing: 10
                columnSpacing: 20
                
                Text { text: "CPU Utilization:"; color: "#a6adc8" }
                Text { 
                    text: processViewModel.cpuUtilization.toFixed(2) + "%"
                    color: "#a6e3a1"
                    font.bold: true 
                }
                
                Text { text: "Effective Time:"; color: "#a6adc8" }
                Text { 
                    text: ganttViewModel.effectiveTime + " ticks"
                    color: "#cdd6f4"
                    font.bold: true 
                }
                
                Text { text: "Idle Time:"; color: "#a6adc8" }
                Text { 
                    text: ganttViewModel.idleTime + " ticks"
                    color: "#fab387"
                    font.bold: true 
                }
                
                Text { text: "Context Switch Time:"; color: "#a6adc8" }
                Text { 
                    text: ganttViewModel.contextSwitchTime + " ticks"
                    color: "#f38ba8"
                    font.bold: true 
                }
                
                Text { text: "Total Context Switches:"; color: "#a6adc8" }
                Text { 
                    text: processViewModel.totalContextSwitches
                    color: "#cba6f7"
                    font.bold: true 
                }
            }
        }
        
        // Memory Stats
        GroupBox {
            title: "Memory Statistics"
            Layout.fillWidth: true
            background: Rectangle { color: "transparent"; border.color: "#313244"; radius: 4 }
            label: Text { text: parent.title; color: "#89b4fa"; font.bold: true }
            
            GridLayout {
                columns: 2
                rowSpacing: 10
                columnSpacing: 20
                
                Text { text: "Final Occupancy:"; color: "#a6adc8" }
                Text { 
                    text: memoryViewModel.memoryOccupancy.toFixed(2) + "%"
                    color: "#a6e3a1"
                    font.bold: true 
                }
                
                Text { text: "Hit Ratio:"; color: "#a6adc8" }
                Text { 
                    text: memoryViewModel.hitRatio.toFixed(2) + "%"
                    color: "#f9e2af"
                    font.bold: true 
                }
                
                Text { text: "Page Faults:"; color: "#a6adc8" }
                Text { 
                    text: memoryViewModel.totalPageFaults
                    color: "#f38ba8"
                    font.bold: true 
                }
                
                Text { text: "Replacements:"; color: "#a6adc8" }
                Text { 
                    text: memoryViewModel.totalReplacements
                    color: "#eba0ac"
                    font.bold: true 
                }
            }
        }
    }
}
