import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15

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
            
            // --- Header Moderno ---
            Rectangle {
                Layout.fillWidth: true
                height: 70
                color: "transparent"
                
                RowLayout {
                    anchors.fill: parent
                    
                    // Logo / Title
                    Rectangle {
                        width: 60
                        height: 60
                        radius: 15
                        color: mainWindow.accentSecondary
                        
                        Text {
                            anchors.centerIn: parent
                            text: "OS"
                            font.bold: true
                            font.pixelSize: 24
                            color: "#11111b"
                        }
                    }
                    
                    Column {
                        Text {
                            text: "WaOS Simulator"
                            font.pixelSize: 28
                            font.bold: true
                            color: mainWindow.textColor
                        }
                        Text {
                            text: "Advanced Scheduling & Memory Visualization"
                            font.pixelSize: 14
                            color: mainWindow.textMuted
                        }
                    }
                    
                    Item { Layout.fillWidth: true } // Spacer
                    
                    // Metrics Summary (Placeholder for top right stats)
                    RowLayout {
                        spacing: 15
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
            
            // --- Control Panel (Config) ---
            ControlPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
            }
            
            // --- Main Content Area ---
            // Styled Tab Bar
            TabBar {
                id: tabBar
                Layout.fillWidth: true
                background: Rectangle { color: "transparent" }
                
                Repeater {
                    model: ["Dashboard", "Memory Map", "System Logs"]
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
            
            // Content Stack
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
                    
                    // View 1: Process Monitor (Dashboard)
                    ProcessMonitor {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    
                    // View 2: Memory Monitor
                    MemoryMonitor {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                    
                    // View 3: Logs
                    ExecutionLog {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }
            
            // --- Blocking Panel (Bottom) ---
            BlockingPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: 180
            }
        }

        // --- Floating Control Bar ---
        Rectangle {
            id: floatingControls
            width: 500
            height: 80
            radius: 40
            color: mainWindow.bgSurface
            border.color: mainWindow.borderColor
            border.width: 1
            
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 40
            anchors.horizontalCenter: parent.horizontalCenter
            
            // Shadow (simulated)
            Rectangle {
                anchors.fill: parent
                anchors.topMargin: 4
                anchors.leftMargin: 4
                z: -1
                radius: 40
                color: "#40000000"
            }

            RowLayout {
                anchors.centerIn: parent
                spacing: 25

                // Reset
                RoundButton {
                    Layout.preferredWidth: 45
                    Layout.preferredHeight: 45
                    onClicked: simulationController.reset()
                    
                    background: Rectangle {
                        radius: 22.5
                        color: parent.down ? mainWindow.bgCard : "transparent"
                        border.color: mainWindow.borderColor
                        border.width: 1
                    }
                    
                    contentItem: Image {
                        source: "qrc:/icons/reset.svg"
                        anchors.centerIn: parent
                        width: 20; height: 20
                        fillMode: Image.PreserveAspectFit
                        // Fallback if icon missing (user will add later)
                        sourceSize.width: 20
                        sourceSize.height: 20
                    }
                }

                // Step
                RoundButton {
                    Layout.preferredWidth: 45
                    Layout.preferredHeight: 45
                    enabled: !simulationController.isRunning
                    onClicked: simulationController.step()
                    
                    background: Rectangle {
                        radius: 22.5
                        color: parent.down ? mainWindow.bgCard : "transparent"
                        border.color: mainWindow.borderColor
                        border.width: 1
                        opacity: parent.enabled ? 1.0 : 0.5
                    }
                    
                    contentItem: Image {
                        source: "qrc:/icons/step.svg"
                        anchors.centerIn: parent
                        width: 20; height: 20
                        fillMode: Image.PreserveAspectFit
                    }
                }

                // Play/Pause (Hero Button)
                RoundButton {
                    Layout.preferredWidth: 65
                    Layout.preferredHeight: 65
                    highlighted: true
                    onClicked: {
                        if (simulationController.isRunning) simulationController.stop()
                        else simulationController.start()
                    }
                    
                    background: Rectangle {
                        radius: 32.5
                        color: mainWindow.accentColor
                        
                        // Glow effect
                        Rectangle {
                            anchors.fill: parent
                            radius: 32.5
                            color: mainWindow.accentColor
                            opacity: 0.3
                            scale: 1.1
                            z: -1
                        }
                    }
                    
                    contentItem: Image {
                        source: simulationController.isRunning ? "qrc:/icons/pause.svg" : "qrc:/icons/play.svg"
                        anchors.centerIn: parent
                        width: 28; height: 28
                        fillMode: Image.PreserveAspectFit
                    }
                }

                // Speed Control
                RowLayout {
                    spacing: 10
                    Text { text: "Speed"; color: mainWindow.textMuted; font.pixelSize: 12 }
                    Slider {
                        from: 100
                        to: 2000
                        value: simulationController.tickInterval
                        onMoved: simulationController.tickInterval = value
                        Layout.preferredWidth: 100
                        
                        background: Rectangle {
                            x: parent.leftPadding
                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                            implicitWidth: 200
                            implicitHeight: 4
                            width: parent.availableWidth
                            height: implicitHeight
                            radius: 2
                            color: mainWindow.bgCard
                            
                            Rectangle {
                                width: parent.parent.visualPosition * parent.width
                                height: parent.height
                                color: mainWindow.accentColor
                                radius: 2
                            }
                        }
                        
                        handle: Rectangle {
                            x: parent.leftPadding + parent.visualPosition * (parent.availableWidth - width)
                            y: parent.topPadding + parent.availableHeight / 2 - height / 2
                            implicitWidth: 16
                            implicitHeight: 16
                            radius: 8
                            color: mainWindow.accentColor
                            border.color: mainWindow.bgDark
                            border.width: 2
                        }
                    }
                }
            }
        }
    }
}
