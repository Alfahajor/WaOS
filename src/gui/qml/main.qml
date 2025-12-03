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

    // --- Theme Palette (WaOS Theme) ---
    property color bgDark: "#121216"
    property color bgSurface: "#121216" // Same as bgDark for seamless look
    property color bgLighter: "#1e1e24" // For Header and Cards
    property color bgCard: "#1e1e24"    // Deprecated alias, pointing to Lighter
    property color accentColor: "#6c5ce7" // Violet
    property color accentSecondary: "#a29bfe" // Light Violet
    property color successColor: "#00b894" // Green
    property color errorColor: "#d63031" // Red
    property color warningColor: "#fdcb6e" // Yellow
    property color textColor: "#dfe6e9"
    property color textMuted: "#636e72"
    property color borderColor: "#2d3436"

    // Selected IDs (Persistent across ticks)
    property int selectedPid: -1
    property int selectedFrameId: -1
    
    // Selection Mode: "process" or "frame"
    property string selectionMode: "process"

    // Update ViewModel when selection changes
    onSelectedFrameIdChanged: {
        var f = getSelectedFrame();
        if (f && f.pid !== -1) {
            memoryViewModel.selectedPid = f.pid;
        } else {
            memoryViewModel.selectedPid = -1;
        }
    }

    // Helper to find the current object instance for the selected PID
    function getSelectedProcess() {
        if (selectedPid === -1) return null;
        var list = processViewModel.processList;
        for (var i = 0; i < list.length; i++) {
            if (list[i].pid === selectedPid) return list[i];
        }
        return null;
    }

    // Helper to find the current object instance for the selected Frame ID
    function getSelectedFrame() {
        if (selectedFrameId === -1) return null;
        var list = memoryViewModel.frameList;
        for (var i = 0; i < list.length; i++) {
            if (list[i].frameId === selectedFrameId) return list[i];
        }
        return null;
    }

    Rectangle {
        anchors.fill: parent
        color: mainWindow.bgDark
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15
            
            // --- Header Section ---
            Rectangle {
                Layout.fillWidth: true
                height: 60
                color: mainWindow.bgLighter // Lighter background for Header
                radius: 8
                    
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
                        
                        // Navigation Buttons (Header)
                        RowLayout {
                            Layout.leftMargin: 40
                            spacing: 10
                            
                            Button {
                                text: "Dashboard"
                                font.bold: true
                                background: Rectangle {
                                    color: tabBar.currentIndex === 0 ? mainWindow.accentColor : "transparent"
                                    radius: 4
                                    border.color: mainWindow.accentColor
                                    border.width: 1
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: tabBar.currentIndex === 0 ? "#11111b" : mainWindow.textColor
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.bold: true
                                }
                                onClicked: tabBar.currentIndex = 0
                            }
                            
                            Button {
                                text: "Memory"
                                font.bold: true
                                background: Rectangle {
                                    color: tabBar.currentIndex === 1 ? mainWindow.accentSecondary : "transparent"
                                    radius: 4
                                    border.color: mainWindow.accentSecondary
                                    border.width: 1
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: tabBar.currentIndex === 1 ? "#11111b" : mainWindow.textColor
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    font.bold: true
                                }
                                onClicked: tabBar.currentIndex = 1
                            }
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
                                    id: settingsButton
                                    icon.source: "qrc:/icons/settings.svg"
                                    icon.color: mainWindow.textColor
                                    icon.width: 20; icon.height: 20
                                    display: AbstractButton.IconOnly
                                    
                                    background: Rectangle { 
                                        color: "transparent" 
                                        radius: 4
                                        border.color: settingsButton.hovered ? mainWindow.accentColor : "transparent"
                                    }
                                    
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
                                    id: stepButton
                                    icon.source: "qrc:/icons/step.svg"
                                    icon.color: enabled ? mainWindow.textColor : mainWindow.textMuted
                                    display: AbstractButton.IconOnly
                                    
                                    background: Rectangle { 
                                        radius: 4
                                        color: stepButton.down ? mainWindow.accentColor : 
                                               (stepButton.hovered ? Qt.rgba(1, 1, 1, 0.1) : "transparent")
                                        border.color: stepButton.down ? mainWindow.accentColor : "transparent"
                                        border.width: 1
                                        
                                        Behavior on color { ColorAnimation { duration: 50 } }
                                    }

                                    enabled: !simulationController.isRunning
                                    onClicked: simulationController.step()
                                    
                                    ToolTip.visible: hovered
                                    ToolTip.text: "Step: Execute 1 Tick"
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
                    implicitWidth: 12 // Wider gap
                    color: "transparent" // Invisible handle to create a gap
                }

                // Item 1: Left Container
                Rectangle {
                    color: "transparent"
                    SplitView.preferredWidth: parent.width * 0.7
                    
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10

                        // Hidden TabBar (Controlled by Header)
                        TabBar {
                            id: tabBar
                            visible: false
                            Layout.fillWidth: true
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

                        // Gantt Chart (Timeline)
                        GanttChart {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 100
                        }

                        // Blocked Processes (Bottom Left)
                        BlockedProcesses {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 160 // Reduced height
                        }
                    }
                }

                // Item 2: Right Container
                Rectangle {
                    color: mainWindow.bgLighter // Lighter background for the container
                    SplitView.preferredWidth: parent.width * 0.3

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        // 1. Inspector Section
                        CollapsibleSection {
                            title: (mainWindow.selectionMode === "frame" && memoryViewModel.selectedPid !== -1) 
                                   ? "Process " + memoryViewModel.selectedPid + " Memory Map"
                                   : "INSPECTOR"
                            Layout.fillWidth: true
                            // Allow it to shrink if needed, but prefer implicit height
                            
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 10
                                
                                // --- Standard Inspector (Process or Empty Frame) ---
                                Item {
                                    visible: !(mainWindow.selectionMode === "frame" && memoryViewModel.selectedPid !== -1)
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 160 // Fixed height for content
                                    
                                    ColumnLayout {
                                        anchors.fill: parent
                                        spacing: 10
                                        
                                        GridLayout {
                                            columns: 2
                                            rowSpacing: 10
                                            columnSpacing: 20
                                            
                                            // Dynamic Labels based on Selection Mode
                                            Label { text: mainWindow.selectionMode === "process" ? "PID:" : "Frame:"; color: mainWindow.textMuted }
                                            Label { 
                                                text: {
                                                    if (mainWindow.selectionMode === "process") {
                                                        var p = mainWindow.getSelectedProcess();
                                                        return p ? p.pid : "-";
                                                    } else {
                                                        var f = mainWindow.getSelectedFrame();
                                                        return f ? "F" + f.frameId : "-";
                                                    }
                                                }
                                                color: mainWindow.textColor; font.bold: true 
                                            }
                                            
                                            Label { text: mainWindow.selectionMode === "process" ? "Priority:" : "Owner PID:"; color: mainWindow.textMuted }
                                            Label { 
                                                text: {
                                                    if (mainWindow.selectionMode === "process") {
                                                        var p = mainWindow.getSelectedProcess();
                                                        return p ? p.priority : "-";
                                                    } else {
                                                        var f = mainWindow.getSelectedFrame();
                                                        return f ? (f.pid === -1 ? "Free" : f.pid) : "-";
                                                    }
                                                }
                                                color: mainWindow.textColor; font.bold: true 
                                            }
                                            
                                            Label { text: mainWindow.selectionMode === "process" ? "State:" : "Page:"; color: mainWindow.textMuted }
                                            Label { 
                                                text: {
                                                    if (mainWindow.selectionMode === "process") {
                                                        var p = mainWindow.getSelectedProcess();
                                                        return p ? p.state : "-";
                                                    } else {
                                                        var f = mainWindow.getSelectedFrame();
                                                        return f ? (f.pid === -1 ? "-" : "P" + f.pid) : "-"; // Mock Page info if not available
                                                    }
                                                }
                                                color: mainWindow.textColor; font.bold: true 
                                            }

                                            Label { text: "Next Page:"; visible: mainWindow.selectionMode === "process"; color: mainWindow.textMuted }
                                            Label { 
                                                visible: mainWindow.selectionMode === "process"
                                                text: {
                                                    var p = mainWindow.getSelectedProcess();
                                                    return p ? p.nextPage : "-";
                                                }
                                                color: mainWindow.textColor; font.bold: true 
                                            }

                                            Label { text: "Ref. String:"; visible: mainWindow.selectionMode === "process"; color: mainWindow.textMuted }
                                            Label { 
                                                visible: mainWindow.selectionMode === "process"
                                                text: {
                                                    var p = mainWindow.getSelectedProcess();
                                                    return p ? p.referenceString : "-";
                                                }
                                                color: mainWindow.textColor; font.bold: true 
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                            }
                                        }
                                        
                                        Label { 
                                            text: mainWindow.selectionMode === "process" ? "Burst Sequence:" : "Content Preview:"; 
                                            color: mainWindow.textMuted; font.pixelSize: 12; topPadding: 10 
                                        }
                                        
                                        // Visual Burst Sequence (Mock for Eye Candy)
                                        ListView {
                                            visible: mainWindow.selectionMode === "process"
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 30
                                            orientation: ListView.Horizontal
                                            spacing: 4
                                            clip: true
                                            
                                            // If no process selected, empty model. Else show mock sequence.
                                            model: mainWindow.getSelectedProcess() ? 6 : 0
                                            
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

                                        // Frame Content Preview (Mock)
                                        Rectangle {
                                            visible: mainWindow.selectionMode === "frame"
                                            Layout.fillWidth: true
                                            Layout.preferredHeight: 30
                                            color: "#2a2a35"
                                            radius: 4
                                            border.color: mainWindow.borderColor
                                            
                                            Text {
                                                anchors.centerIn: parent
                                                text: {
                                                    var f = mainWindow.getSelectedFrame();
                                                    return (f && f.pid !== -1) ? "0x" + (f.frameId * 12345).toString(16).toUpperCase() : "Empty";
                                                }
                                                color: "#a6adc8"
                                                font.family: "Courier New"
                                            }
                                        }
                                        
                                        Item { Layout.fillHeight: true } // Spacer
                                    }
                                }

                                // --- Page Table View (Frame Selected & Valid PID) ---
                                ListView {
                                    visible: (mainWindow.selectionMode === "frame" && memoryViewModel.selectedPid !== -1)
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 160
                                    clip: true
                                    model: memoryViewModel.pageTable
                                    spacing: 2

                                    header: RowLayout {
                                        width: parent.width
                                        spacing: 10
                                        Text { text: "Page"; color: mainWindow.textMuted; font.bold: true; Layout.preferredWidth: 40 }
                                        Text { text: "Frame"; color: mainWindow.textMuted; font.bold: true; Layout.preferredWidth: 50 }
                                        Text { text: "Status"; color: mainWindow.textMuted; font.bold: true; Layout.fillWidth: true }
                                    }

                                    delegate: Rectangle {
                                        width: ListView.view.width
                                        height: 24
                                        color: index % 2 === 0 ? "transparent" : "#1e1e2e"
                                        radius: 4

                                        RowLayout {
                                            anchors.fill: parent
                                            anchors.leftMargin: 5
                                            spacing: 10
                                            
                                            Text { 
                                                text: model.pageNumber
                                                color: mainWindow.textColor
                                                font.family: "Consolas"
                                                Layout.preferredWidth: 40
                                            }
                                            
                                            Text { 
                                                text: model.frameNumber
                                                color: mainWindow.accentColor
                                                font.bold: true
                                                Layout.preferredWidth: 50
                                            }
                                            
                                            Text { 
                                                text: model.state
                                                // Green if Valid (frameNumber != "—"), Gray otherwise
                                                color: model.frameNumber !== "—" ? mainWindow.successColor : mainWindow.textMuted
                                                font.bold: true
                                                Layout.fillWidth: true
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // 2. System Health Section
                        CollapsibleSection {
                            title: "SYSTEM HEALTH"
                            Layout.fillWidth: true
                            
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 10
                                
                                RowLayout {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 100
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
                                            property real value: memoryViewModel.hitRatio
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
                                                text: memoryViewModel.hitRatio.toFixed(1) + "%"
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

                                // Memory Stats Text
                                RowLayout {
                                    Layout.fillWidth: true
                                    Layout.topMargin: 5
                                    spacing: 20
                                    
                                    Column {
                                        Layout.fillWidth: true
                                        Layout.alignment: Qt.AlignHCenter
                                        Text { 
                                            text: "Page Faults"
                                            color: mainWindow.textMuted
                                            font.pixelSize: 11
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                        Text { 
                                            text: memoryViewModel.totalPageFaults
                                            color: mainWindow.errorColor // Red/Pink
                                            font.bold: true
                                            font.pixelSize: 16
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                    }
                                    
                                    Column {
                                        Layout.fillWidth: true
                                        Layout.alignment: Qt.AlignHCenter
                                        Text { 
                                            text: "Replacements"
                                            color: mainWindow.textMuted
                                            font.pixelSize: 11
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                        Text { 
                                            text: memoryViewModel.totalReplacements
                                            color: mainWindow.accentSecondary // Purple
                                            font.bold: true
                                            font.pixelSize: 16
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                    }
                                }

                                // CPU Stats Text (CS / Idle)
                                RowLayout {
                                    Layout.fillWidth: true
                                    Layout.topMargin: 5
                                    spacing: 20
                                    
                                    Column {
                                        Layout.fillWidth: true
                                        Layout.alignment: Qt.AlignHCenter
                                        Text { 
                                            text: "Context Switches"
                                            color: mainWindow.textMuted
                                            font.pixelSize: 11
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                        Text { 
                                            text: processViewModel.totalContextSwitches
                                            color: mainWindow.warningColor // Orange
                                            font.bold: true
                                            font.pixelSize: 16
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                    }
                                    
                                    Column {
                                        Layout.fillWidth: true
                                        Layout.alignment: Qt.AlignHCenter
                                        Text { 
                                            text: "Idle Time"
                                            color: mainWindow.textMuted
                                            font.pixelSize: 11
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                        Text { 
                                            text: ganttViewModel.idleTime + " ticks"
                                            color: "#b2bec3" // Gray
                                            font.bold: true
                                            font.pixelSize: 16
                                            anchors.horizontalCenter: parent.horizontalCenter
                                        }
                                    }
                                }
                            }
                        }

                        // 3. Execution Log (Fills remaining space)
                        CollapsibleSection {
                            title: "EXECUTION LOG"
                            Layout.fillWidth: true
                            Layout.fillHeight: true // This one expands to fill space
                            
                            ExecutionLog {
                                anchors.fill: parent
                            }
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

        // Summary Dialog Instance
        SummaryDialog {
            id: summaryDialog
            anchors.centerIn: parent
        }

        Connections {
            target: simulationController
            function onSimulationFinished() {
                summaryDialog.open()
            }
        }
    }
}
