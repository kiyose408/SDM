import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: root
    width: 800
    height: 600
    visible: true
    title: "SmartDietManager"
    color: "#F5F5F5"

    // Placeholder — will be replaced by real navigation in Task 1.4
    Text {
        anchors.centerIn: parent
        text: "🥗 SmartDietManager"
        font.pixelSize: 24
        color: "#333333"
    }
}
