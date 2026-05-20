import QtQuick
import QtQuick.Controls
import QtQuick.Window
import SmartDiet.Style 1.0

ApplicationWindow {
    id: root
    width: 800
    height: 600
    visible: true
    title: "SmartDietManager"
    color: Theme.bgPage

    Text {
        anchors.centerIn: parent
        text: "🥗 SmartDietManager"
        font.pixelSize: Theme.fontSizeLarge
        color: Theme.textPrimary
    }
}
