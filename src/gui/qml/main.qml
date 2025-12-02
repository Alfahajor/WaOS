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

    // Selected Process for Inspector
    property var selectedProcess: null

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
                            Layout.preferredHeight: 220
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

                        // 1. Inspector Section
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 200
                            color: mainWindow.bgCard
                            radius: 8
                            border.color: mainWindow.borderColor

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 10
                                
                                Label { text: "INSPECTOR"; font.bold: true; color: mainWindow.textMuted }
                                
                                GridLayout {
                                    columns: 2
                                    rowSpacing: 10
                                    columnSpacing: 20
                                    
                                    Label { text: "PID:"; color: mainWindow.textMuted }
                                    Label { 
                                        text: mainWindow.selectedProcess ? mainWindow.selectedProcess.pid : "-"
                                        color: mainWindow.textColor; font.bold: true 
                                    }
                                    
                                    Label { text: "Priority:"; color: mainWindow.textMuted }
                                    Label { 
                                        text: mainWindow.selectedProcess ? mainWindow.selectedProcess.priority : "-"
                                        color: mainWindow.textColor; font.bold: true 
                                    }
                                    
                                    Label { text: "State:"; color: mainWindow.textMuted }
                                    Label { 
                                        text: mainWindow.selectedProcess ? mainWindow.selectedProcess.state : "-"
                                        color: mainWindow.textColor; font.bold: true 
                                    }
                                }
                                
                                Label { text: "Burst Sequence:"; color: mainWindow.textMuted; font.pixelSize: 12; topPadding: 10 }
                                
                                // Visual Burst Sequence (Mock for Eye Candy)
                                ListView {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 30
                                    orientation: ListView.Horizontal
                                    spacing: 4
                                    clip: true
                                    
                                    // If no process selected, empty model. Else show mock sequence.
                                    model: mainWindow.selectedProcess ? 6 : 0
                                    
                                    delegate: Rectangle {
                                        width: index % 2 == 0 ? 40 : 20 // CPU longer than IO
                                        height: 20
                                        radius: 3
                                        // Alternate colors: CPU (Blue), IO (Yellow)
                                        color: index % 2 == 0 ? mainWindow.accentColor : "#f9e2af"
                                        opacity: 0.8
                                        
                                        Text {
                                            anchors.centerIn: parent
                                            text: index % 2 == 0 ? "CPU" : "IO"
                                            font.pixelSize: 9
                                            font.bold: true
                                            color: "#11111b"
                                        }
                                    }
                                }
                                
                                Item { Layout.fillHeight: true } // Spacer
                            }
                        }

                        // 2. System Health Section
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 160 // Increased height for Gauges
                            color: mainWindow.bgCard
                            radius: 8
                            border.color: mainWindow.borderColor

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 15
                                spacing: 10
                                
                                Label { text: "SYSTEM HEALTH"; font.bold: true; color: mainWindow.textMuted }
                                
                                RowLayout {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    spacing: 20
                                    
                                    // CPU Gauge
                                    Item {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        
                                        Canvas {
                                            id: cpuCanvas
                                            anchors.centerIn: parent
                                            width: 80; height: 80
                                            property real value: processViewModel.cpuUtilization
                                            onValueChanged: requestPaint()
                                            
                                            onPaint: {
                                                var ctx = getContext("2d");
                                                ctx.reset();
                                                var center = width / 2;
                                                var radius = width / 2 - 5;
                                                
                                                // Background Ring
                                                ctx.beginPath();
                                                ctx.arc(center, center, radius, 0, 2 * Math.PI);
                                                ctx.lineWidth = 8;
                                                ctx.strokeStyle = "#313244";
                                                ctx.stroke();
                                                
                                                // Progress Arc
                                                var startAngle = -Math.PI / 2;
                                                var endAngle = startAngle + (value / 100) * 2 * Math.PI;
                                                ctx.beginPath();
                                                ctx.arc(center, center, radius, startAngle, endAngle);
                                                ctx.lineWidth = 8;
                                                ctx.strokeStyle = mainWindow.accentColor; // Cyan
                                                ctx.lineCap = "round";
                                                ctx.stroke();
                                            }
                                        }
                                        
                                        Column {
                                            anchors.centerIn: parent
                                            Text { 
                                                text: Math.round(processViewModel.cpuUtilization) + "%"
                                                color: mainWindow.textColor
                                                font.bold: true
                                                font.pixelSize: 16
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                            Text { 
                                                text: "CPU"
                                                color: mainWindow.textMuted
                                                font.pixelSize: 10
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                        }
                                    }
                                    
                                    // Hit Ratio Gauge
                                    Item {
                                        Layout.fillWidth: true
                                        Layout.fillHeight: true
                                        
                                        Canvas {
                                            id: hitCanvas
                                            anchors.centerIn: parent
                                            width: 80; height: 80
                                            property real value: 85 // Mock value as backend returns "-"
                                            onValueChanged: requestPaint()
                                            
                                            onPaint: {
                                                var ctx = getContext("2d");
                                                ctx.reset();
                                                var center = width / 2;
                                                var radius = width / 2 - 5;
                                                
                                                // Background Ring
                                                ctx.beginPath();
                                                ctx.arc(center, center, radius, 0, 2 * Math.PI);
                                                ctx.lineWidth = 8;
                                                ctx.strokeStyle = "#313244";
                                                ctx.stroke();
                                                
                                                // Progress Arc
                                                var startAngle = -Math.PI / 2;
                                                var endAngle = startAngle + (value / 100) * 2 * Math.PI;
                                                ctx.beginPath();
                                                ctx.arc(center, center, radius, startAngle, endAngle);
                                                ctx.lineWidth = 8;
                                                ctx.strokeStyle = mainWindow.successColor; // Green
                                                ctx.lineCap = "round";
                                                ctx.stroke();
                                            }
                                        }
                                        
                                        Column {
                                            anchors.centerIn: parent
                                            Text { 
                                                text: "85%" // Mock
                                                color: mainWindow.textColor
                                                font.bold: true
                                                font.pixelSize: 16
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                            Text { 
                                                text: "Hit Ratio"
                                                color: mainWindow.textMuted
                                                font.pixelSize: 10
                                                anchors.horizontalCenter: parent.horizontalCenter
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // 3. Execution Log (Fills remaining space)
                        ExecutionLog {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
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
