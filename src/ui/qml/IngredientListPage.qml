import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "食材库"

    background: Rectangle { color: Theme.bgPage }

    // ingredientService 由 main.cpp setContextProperty 注入

    ListView {
        id: listView
        anchors { fill: parent; margins: Theme.spacingMedium }
        spacing: Theme.spacingSmall
        model: ListModel { id: ingredientModel }

        header: Text {
            text: "食材库 (" + ingredientModel.count + ")"
            font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary
        }

        delegate: Rectangle {
            width: ListView.view.width - Theme.spacingMedium * 2; height: 56
            radius: Theme.radiusSmall; color: Theme.bgCard
            border { width: 1; color: Theme.borderLight }

            Row {
                anchors { left: parent.left; leftMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
                spacing: Theme.spacingSmall
                Column {
                    Text { text: model.name; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
                    Text { text: model.category + " · " + model.caloriesPer100 + " kcal/100" + model.unit; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
                }
            }
            Text { text: ">"; font.pixelSize: Theme.fontSizeBody; color: Theme.textHint; anchors { right: parent.right; rightMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter } }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var comp = Qt.createComponent("qrc:/qml/IngredientDetailPage.qml")
                    if (comp.status === Component.Ready) {
                        var page = comp.createObject(navStack, { ingredientId: model.id })
                        if (page) navStack.push(page)
                    }
                }
            }
        }

        Component.onCompleted: refresh()
    }

    function refresh() {
        ingredientModel.clear()
        var items = ingredientService.getAll()
        for (var i = 0; i < items.length; i++) ingredientModel.append(items[i])
    }
}
