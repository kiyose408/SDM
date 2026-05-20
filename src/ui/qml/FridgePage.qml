import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "冰箱"

    Rectangle {
        anchors.fill: parent
        color: Theme.bgPage

        Column {
            anchors.centerIn: parent
            spacing: Theme.spacingMedium

            Icon {
                name: "kitchen"
                size: 48
                color: Theme.primary
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: "家庭共享冰箱"
                font.pixelSize: Theme.fontSizeTitle
                color: Theme.textPrimary
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
