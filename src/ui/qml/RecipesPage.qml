import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "菜谱"

    background: Rectangle { color: Theme.bgPage }

    Column {
        anchors.centerIn: parent
        spacing: Theme.spacingLarge

        Icon { name: "menu_book"; size: 56; color: Theme.primary; anchors.horizontalCenter: parent.horizontalCenter }
        Text { text: "菜谱与食材"; font.pixelSize: Theme.fontSizeLarge; color: Theme.textPrimary; anchors.horizontalCenter: parent.horizontalCenter }

        Item { width: 1; height: Theme.spacingMedium }

        Rectangle {
            width: 220; height: 52; radius: Theme.radiusLarge; color: Theme.primary
            Text { anchors.centerIn: parent; text: "📖 菜谱列表"; font.pixelSize: Theme.fontSizeBody; color: "white" }
            anchors.horizontalCenter: parent.horizontalCenter
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var c = Qt.createComponent("qrc:/qml/RecipeListPage.qml")
                    if (c.status === Component.Ready) navStack.push(c.createObject(navStack))
                }
            }
        }

        Rectangle {
            width: 220; height: 52; radius: Theme.radiusLarge
            color: "transparent"; border { width: 1; color: Theme.primary }
            Text { anchors.centerIn: parent; text: "🥬 食材库"; font.pixelSize: Theme.fontSizeBody; color: Theme.primary }
            anchors.horizontalCenter: parent.horizontalCenter
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var c = Qt.createComponent("qrc:/qml/IngredientListPage.qml")
                    if (c.status === Component.Ready) navStack.push(c.createObject(navStack))
                }
            }
        }

        Rectangle {
            width: 220; height: 52; radius: Theme.radiusLarge; color: Theme.secondary
            Text { anchors.centerIn: parent; text: "+ 创建菜谱"; font.pixelSize: Theme.fontSizeBody; color: "white" }
            anchors.horizontalCenter: parent.horizontalCenter
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log("Create recipe clicked")
                    var c = Qt.createComponent("qrc:/qml/RecipeEditPage.qml")
                    console.log("Component status:", c.status, c.errorString())
                    if (c.status === Component.Ready) {
                        var fams = familyService.getUserFamilies(session.userId)
                        var fid = fams.length > 0 ? fams[0].familyId : ""
                        navStack.push(c.createObject(navStack, { familyId: fid }))
                    }
                }
            }
        }
    }
}
