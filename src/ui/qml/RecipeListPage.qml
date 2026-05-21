import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root; title: "菜谱列表"
    background: Rectangle { color: Theme.bgPage }

    Item {
        anchors { top: parent.top; topMargin: Theme.spacingSmall; left: parent.left; leftMargin: Theme.spacingSmall }
        width: backRow.width; height: backRow.height
        Row { id: backRow; spacing: 4
            Icon { name: "arrow_back"; size: 24; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "返回"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
        }
        MouseArea { anchors.fill: parent; onClicked: navStack.pop() }
    }

    // 搜索栏
    Rectangle {
        anchors { top: parent.top; topMargin: 40; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium }
        height: 40; radius: Theme.radiusLarge; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
        Row {
            anchors { left: parent.left; leftMargin: Theme.spacingSmall; verticalCenter: parent.verticalCenter }
            spacing: Theme.spacingSmall
            Icon { name: "search"; size: 20; color: Theme.textHint; anchors.verticalCenter: parent.verticalCenter }
            TextInput {
                id: searchInput
                width: parent.parent.width - 80
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary
                Text {
                    text: "搜索菜谱..."
                    font: parent.font; color: Theme.textHint
                    visible: !searchInput.text && !searchInput.activeFocus
                }
                onTextChanged: refresh()
            }
        }
    }

    ListView {
        anchors { top: parent.top; topMargin: 90; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: parent.bottom }
        spacing: Theme.spacingSmall
        model: ListModel { id: model }
        header: Text {
            text: "共 " + model.count + " 道菜谱"
            font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint
        }
        delegate: Rectangle {
            width: parent.width; height: 56; radius: Theme.radiusSmall
            color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
            Row {
                anchors { left: parent.left; leftMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
                spacing: Theme.spacingSmall
                Column {
                    Text { text: model.name; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
                    Text { text: model.calPer100.toFixed(0)+" kcal/100g · "+model.cookingTime+" min"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
                }
            }
            Text { text: ">"; font.pixelSize: Theme.fontSizeBody; color: Theme.textHint; anchors { right: parent.right; rightMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter } }
            MouseArea { anchors.fill: parent; onClicked: {
                var c=Qt.createComponent("qrc:/qml/RecipeDetailPage.qml")
                if(c.status===Component.Ready) navStack.push(c.createObject(navStack,{recipeId:model.id}))
            }}
        }
        Component.onCompleted: refresh()
    }

    onVisibleChanged: { if (visible) refresh() }

    function refresh() {
        model.clear()
        var kw = searchInput.text.trim()
        var items = kw ? recipeService.searchByName(kw) : recipeService.getAll()
        for (var i = 0; i < items.length; i++) model.append(items[i])
    }
}
