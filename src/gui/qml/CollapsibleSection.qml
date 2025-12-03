import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    id: root
    spacing: 0
    
    default property alias content: contentLoader.sourceComponent
    property string title: "Section"
    property bool isExpanded: true
    property color headerColor: "transparent"
    property color contentBackgroundColor: "#11111b" // Darker background for content
    property color borderColor: "#2d3436"

    // Header
    Rectangle {
        Layout.fillWidth: true
        height: 35
        color: root.headerColor
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 10
            
            // Arrow Icon
            Image {
                source: root.isExpanded ? "qrc:/icons/chevron_down.svg" : "qrc:/icons/chevron_right.svg"
                sourceSize.width: 16
                sourceSize.height: 16
                Layout.alignment: Qt.AlignVCenter
                
                // Simple color filter trick if needed, or rely on SVG color
                // For now, assuming SVG has currentColor or white
            }
            
            // Title
            Text {
                text: root.title
                color: "#dfe6e9"
                font.bold: true
                font.pixelSize: 14
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
            }
        }
        
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.isExpanded = !root.isExpanded
        }
    }

    // Content Container
    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: root.isExpanded // Fill available height if expanded (for Log)
        Layout.preferredHeight: root.isExpanded ? implicitHeight : 0
        visible: root.isExpanded
        color: root.contentBackgroundColor
        clip: true
        
        // Border only on sides and bottom if needed, or just a container
        border.color: root.borderColor
        border.width: 1
        radius: 4
        
        // Padding for content
        Loader {
            id: contentLoader
            anchors.fill: parent
            anchors.margins: 10
        }
    }
}
