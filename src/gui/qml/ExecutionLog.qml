import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "transparent"
    clip: true

    // Theme Colors
    property color textColor: "#ffffff" // Brighter white
    property color logColor: "#a6e3a1" 
    property color bgConsole: "transparent" // Background handled by parent container
    property color accentColor: "#89b4fa"
    property color errorColor: "#f38ba8"
    property color warnColor: "#fab387"
    property color successColor: "#a6e3a1"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Log List
        ListView {
            id: logListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: executionLogViewModel
            clip: true
            spacing: 0 
            
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
                    Layout.topMargin: 6
                    Layout.bottomMargin: 6
                    spacing: 10

                    // Time
                    Text {
                        text: "00:" + (index % 60 < 10 ? "0" + (index % 60) : (index % 60))
                        color: "#bbbbbb" // Slightly brighter gray
                        font.family: "Consolas"
                        font.pixelSize: 12
                    }

                    // Tag
                    Rectangle {
                        width: tagText.implicitWidth + 12
                        height: 20
                        color: parsed.color
                        radius: 4
                        opacity: 0.2 // Slightly more visible background
                        
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
                        color: "#ffffff" // Pure white for better visibility
                        font.family: "Consolas"
                        font.pixelSize: 13
                        font.weight: Font.Medium // Thicker font
                        wrapMode: Text.Wrap
                    }
                }

                // Separator
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#444444" // Slightly lighter separator
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