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

    StackView {
        id: navStack
        anchors.fill: parent
        initialItem: HomePage {}
    }
}
