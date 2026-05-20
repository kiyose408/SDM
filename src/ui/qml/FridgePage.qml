import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "冰箱"

    Rectangle {
        anchors.fill: parent
        color: Theme.bgPage

        Text {
            anchors.centerIn: parent
            text: "🧊 家庭共享冰箱"
            font.pixelSize: Theme.fontSizeTitle
            color: Theme.textPrimary
        }
    }
}
