import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "transparent"
    clip: true

    // Theme Colors
    property color textColor: "#cdd6f4"
    property color logColor: "#a6e3a1" // Green terminal text
    property color bgConsole: "#11111b" // Darker console background
    property color accentColor: "#89b4fa"
    property color errorColor: "#f38ba8"
    property color warnColor: "#fab387"
    property color successColor: "#a6e3a1"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        Label {
            text: "System Execution Log"
            font.bold: true
            font.pixelSize: 20
            color: textColor
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: bgConsole
            radius: 8
            border.color: "#313244"
            
            ListView {
                id: logListView
                anchors.fill: parent
                anchors.margins: 10
                model: executionLogViewModel
                clip: true
                spacing: 0 // Spacing handled by padding in delegate
                
                delegate: ColumnLayout {
                    width: ListView.view.width
                    spacing: 4

                    // Helper to parse message
                    property var parsed: {
                        var msg = model.message;
                        var tag = "INFO";
                        var content = msg;
                        
                        // Try to extract tag [TAG]
                        var match = msg.match(/^\[(.*?)\]\s*(.*)/);
                        if (match) {
                            tag = match[1];
                            content = match[2];
                        }
                        
                        // Determine color
                        var color = accentColor;
                        var checkStr = (tag + " " + content).toLowerCase();
                        
                        if (checkStr.includes("mem") || checkStr.includes("fault") || checkStr.includes("alloc")) color = successColor;
                        else if (checkStr.includes("i/o") || checkStr.includes("wait") || checkStr.includes("block")) color = warnColor;
                        else if (checkStr.includes("error") || checkStr.includes("fail")) color = errorColor;
                        
                        return { tag: tag, content: content, color: color };
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.topMargin: 4
                        Layout.bottomMargin: 4
                        spacing: 10

                        // Time
                        Text {
                            text: "00:" + (index % 60 < 10 ? "0" + (index % 60) : (index % 60))
                            color: "#888888"
                            font.family: "Consolas"
                            font.pixelSize: 12
                        }

                        // Tag
                        Rectangle {
                            width: tagText.implicitWidth + 12
                            height: 20
                            color: parsed.color
                            radius: 4
                            opacity: 0.15 // Background opacity
                            
                            Text {
                                id: tagText
                                anchors.centerIn: parent
                                text: parsed.tag
                                color: parsed.color
                                font.bold: true
                                font.pixelSize: 11
                            }
                        }

                        // Message
                        Text {
                            Layout.fillWidth: true
                            text: parsed.content
                            color: "#cdd6f4" // White/Light Gray
                            font.family: "Consolas"
                            font.pixelSize: 13
                            wrapMode: Text.Wrap
                        }
                    }

                    // Separator
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#333333"
                        opacity: 0.5
                    }
                }

                // Auto-scroll to bottom
                onCountChanged: {
                    positionViewAtEnd()
                }
            }
        }
    }
}