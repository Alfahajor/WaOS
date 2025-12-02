import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: mainWindow
    visible: true
    width: 1400
    height: 900
    title: "WaOS - Simulator"
    color: "#11111b" // Background Dark

    // --- Theme Palette (Dracula/Catppuccin inspired) ---
    property color bgDark: "#11111b"
    property color bgSurface: "#1e1e2e"
    property color bgCard: "#252535"
    property color accentColor: "#89b4fa" // Blue
    property color accentSecondary: "#cba6f7" // Purple
    property color successColor: "#a6e3a1"
    property color errorColor: "#f38ba8"
    property color textColor: "#cdd6f4"
    property color textMuted: "#a6adc8"
    property color borderColor: "#313244"

    Rectangle {
        anchors.fill: parent
        color: mainWindow.bgDark
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15
            
            // --- Header Section ---
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10

                // Header Moderno
                Rectangle {
                    Layout.fillWidth: true
                    height: 50
                    color: "transparent"
                    
                    RowLayout {
                        anchors.fill: parent
                        
                        // Title Only
                        Text {
                            text: "WaOS - Simulator"
                            font.pixelSize: 24
                            font.bold: true
                            color: mainWindow.textColor
                            Layout.alignment: Qt.AlignVCenter
                        }
                        
                        Item { Layout.fillWidth: true } // Spacer
                        
                        // Metrics Summary
                        RowLayout {
                            spacing: 15
                            
                            // --- Playback Controls ---
                            RowLayout {
                                spacing: 10
                                
                                // Settings Button
                                Button {
                                    text: "⚙️"
                                    font.pixelSize: 20
                                    display: AbstractButton.TextOnly
                                    background: Rectangle { color: "transparent" }
                                    onClicked: settingsDialog.open()
                                    ToolTip.visible: hovered; ToolTip.text: "Settings"
                                }

                                // Reset
                                Button {
                                    icon.source: "qrc:/icons/reset.svg"
                                    icon.color: mainWindow.textMuted
                                    display: AbstractButton.IconOnly
                                    background: Rectangle { color: "transparent" }
                                    onClicked: simulationController.reset()
                                    ToolTip.visible: hovered; ToolTip.text: "Reset"
                                }

                                // Step
                                Button {
                                    icon.source: "qrc:/icons/step.svg"
                                    icon.color: enabled ? mainWindow.textColor : mainWindow.textMuted
                                    display: AbstractButton.IconOnly
                                    background: Rectangle { color: "transparent" }
                                    enabled: !simulationController.isRunning
                                    onClicked: simulationController.step()
                                    ToolTip.visible: hovered; ToolTip.text: "Step"
                                }

                                // Play/Pause
                                Button {
                                    icon.source: simulationController.isRunning ? "qrc:/icons/pause.svg" : "qrc:/icons/play.svg"
                                    icon.color: mainWindow.accentColor
                                    icon.width: 32; icon.height: 32
                                    display: AbstractButton.IconOnly
                                    background: Rectangle { color: "transparent" }
                                    onClicked: {
                                        if (simulationController.isRunning) simulationController.stop()
                                        else simulationController.start()
                                    }
                                }
                                
                                // Speed
                                Slider {
                                    from: 100; to: 2000
                                    value: simulationController.tickInterval
                                    onMoved: simulationController.tickInterval = value
                                    Layout.preferredWidth: 80
                                    background: Rectangle {
                                        implicitHeight: 4; width: parent.width; height: implicitHeight
                                        radius: 2; color: mainWindow.bgCard
                                        Rectangle { width: parent.parent.visualPosition * parent.width; height: parent.height; color: mainWindow.accentColor; radius: 2 }
                                    }
                                    handle: Rectangle {
                                        x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                                        y: parent.topPadding + parent.availableHeight / 2 - height / 2
                                        implicitWidth: 12; implicitHeight: 12; radius: 6
                                        color: mainWindow.accentColor
                                    }
                                }
                            }

                            Rectangle {
                                width: 1; height: 30; color: mainWindow.borderColor
                            }

                            Rectangle {
                                width: 120; height: 50; radius: 8
                                color: mainWindow.bgSurface
                                border.color: mainWindow.borderColor
                                Column {
                                    anchors.centerIn: parent
                                    Text { text: "CPU Util"; color: mainWindow.textMuted; font.pixelSize: 10; anchors.horizontalCenter: parent.horizontalCenter }
                                    Text { text: "0%"; color: mainWindow.successColor; font.bold: true; font.pixelSize: 16; anchors.horizontalCenter: parent.horizontalCenter }
                                }
                            }
                        }
                    }
                }


            }
            
            // --- Main Content (SplitView) ---
            SplitView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                orientation: Qt.Horizontal

                handle: Rectangle {
                    implicitWidth: 4
                    color: SplitView.pressed ? mainWindow.accentColor : mainWindow.borderColor
                }

                // Item 1: Left Container
                Rectangle {
                    color: "transparent"
                    SplitView.preferredWidth: parent.width * 0.7
                    SplitView.fillHeight: true
                    
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10

                        // Tab Bar for Process Monitor / Memory Monitor
                        TabBar {
                            id: tabBar
                            Layout.fillWidth: true
                            background: Rectangle { color: "transparent" }
                            
                            Repeater {
                                model: ["Dashboard", "Memory Map"]
                                TabButton {
                                    text: modelData
                                    width: implicitWidth + 40
                                    
                                    contentItem: Text {
                                        text: parent.text
                                        font.bold: true
                                        color: parent.checked ? mainWindow.accentColor : mainWindow.textMuted
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    
                                    background: Rectangle {
                                        color: parent.checked ? "#2089b4fa" : "transparent"
                                        radius: 5
                                        border.width: 0
                                        
                                        Rectangle {
                                            width: parent.width
                                            height: 3
                                            anchors.bottom: parent.bottom
                                            color: mainWindow.accentColor
                                            visible: parent.parent.checked
                                        }
                                    }
                                }
                            }
                        }

                        // Main View (Process Monitor / Memory Monitor)
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: mainWindow.bgSurface
                            radius: 12
                            border.color: mainWindow.borderColor
                            border.width: 1
                            clip: true

                            StackLayout {
                                anchors.fill: parent
                                anchors.margins: 1
                                currentIndex: tabBar.currentIndex
                                
                                // View 1: Process Monitor
                                ProcessMonitor {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                }
                                
                                // View 2: Memory Monitor
                                MemoryMonitor {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                }
                            }
                        }

                        // Blocked Processes (Bottom Left)
                        BlockedProcesses {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 180
                        }
                    }
                }

                // Item 2: Right Container
                Rectangle {
                    color: "transparent"
                    SplitView.preferredWidth: parent.width * 0.3
                    SplitView.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10

                        // Execution Log
                        ExecutionLog {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }

                        // Unlock Notifications
                        UnlockNotifications {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 200
                        }
                    }
                }
            }
        }

        // Settings Dialog Instance
        SettingsDialog {
            id: settingsDialog
            anchors.centerIn: parent
        }
    }
}
