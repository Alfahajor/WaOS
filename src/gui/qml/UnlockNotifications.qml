import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: bgCard
    radius: 8
    border.color: borderColor
    
    // Theme Colors
    property color bgCard: "#252535"
    property color borderColor: "#313244"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        Label { text: "Unlock Notifications"; font.bold: true; color: "#a6e3a1" } // Green
        
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: blockingViewModel.notifications
            spacing: 2
            
            delegate: Text {
                text: "> " + modelData
                font.pixelSize: 12
                font.family: "Consolas"
                color: "#a6e3a1"
                padding: 2
            }
        }
    }
}
