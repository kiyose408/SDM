import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "菜谱"

    background: Rectangle { color: Theme.bgPage }

    Column {
        anchors { fill: parent; margins: Theme.spacingMedium }
        spacing: Theme.spacingMedium

        Row {
            spacing: Theme.spacingSmall
            Rectangle {
                width: (parent.width - Theme.spacingSmall) / 2; height: 44
                radius: Theme.radiusLarge; color: Theme.primary
                Text { anchors.centerIn: parent; text: "食材库"; font.pixelSize: Theme.fontSizeBody; color: "white" }
                MouseArea { anchors.fill: parent; onClicked: {
                    var c = Qt.createComponent("qrc:/qml/IngredientListPage.qml")
                    if (c.status===Component.Ready) navStack.push(c.createObject(navStack)) }}
            }
            Rectangle {
                width: (parent.width - Theme.spacingSmall) / 2; height: 44
                radius: Theme.radiusLarge; color: Theme.secondary
                Text { anchors.centerIn: parent; text: "菜谱列表"; font.pixelSize: Theme.fontSizeBody; color: "white" }
                MouseArea { anchors.fill: parent; onClicked: {
                    var c = Qt.createComponent("qrc:/qml/RecipeListPage.qml")
                    if (c.status===Component.Ready) navStack.push(c.createObject(navStack)) }}
            }
        }

        Text { text: "今日推荐"; font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary }
        ListView {
            id: rv; width: parent.width; height: parent.height - 100
            spacing: Theme.spacingSmall
            model: ListModel { id: recipeModel }
            delegate: Rectangle {
                width: rv.width; height: 56; radius: Theme.radiusSmall
                color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                Row {
                    anchors { left: parent.left; leftMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
                    spacing: Theme.spacingSmall
                    Column {
                        Text { text: model.name; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
                        Text { text: model.totalCalories.toFixed(0)+" kcal · "+model.cookingTime+" min"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
                    }
                }
            }
            Component.onCompleted: {
                var items = recipeService.getAll()
                for (var i=0;i<items.length;i++) recipeModel.append(items[i])
            }
        }
    }
}
