import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "菜谱"

    background: Rectangle { color: Theme.bgPage }

    Column {
        anchors.centerIn: parent
        spacing: Theme.spacingMedium

        Icon { name: "menu_book"; size: 48; color: Theme.primary; anchors.horizontalCenter: parent.horizontalCenter }
        Text { text: "菜谱与食材"; font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary; anchors.horizontalCenter: parent.horizontalCenter }

        Item { width: 1; height: Theme.spacingLarge }

        Rectangle {
            width: 200; height: 44; radius: Theme.radiusLarge; color: Theme.primary
            Text { anchors.centerIn: parent; text: "食材库"; font.pixelSize: Theme.fontSizeBody; color: "white" }
            anchors.horizontalCenter: parent.horizontalCenter
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var comp = Qt.createComponent("qrc:/qml/IngredientListPage.qml")
                    if (comp.status === Component.Ready) {
                        var page = comp.createObject(navStack, {})
                        if (page) navStack.push(page)
                    }
                }
            }
        }
    }
}
