import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "首页"

    Rectangle {
        anchors.fill: parent
        color: Theme.bgPage

        Text {
            anchors.centerIn: parent
            text: "🏠 今日膳食控制台"
            font.pixelSize: Theme.fontSizeTitle
            color: Theme.textPrimary
        }
    }
}
