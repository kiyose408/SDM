import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "菜谱"

    Rectangle {
        anchors.fill: parent
        color: Theme.bgPage

        Text {
            anchors.centerIn: parent
            text: "📖 菜谱与食材"
            font.pixelSize: Theme.fontSizeTitle
            color: Theme.textPrimary
        }
    }
}
