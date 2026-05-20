import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "我的"

    Rectangle {
        anchors.fill: parent
        color: Theme.bgPage

        Text {
            anchors.centerIn: parent
            text: "👤 个人中心"
            font.pixelSize: Theme.fontSizeTitle
            color: Theme.textPrimary
        }
    }
}
