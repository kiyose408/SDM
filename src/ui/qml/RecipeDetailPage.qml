import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: ""
    property string recipeId: ""
    background: Rectangle { color: Theme.bgPage }

    Component.onCompleted: loadData()
    onVisibleChanged: { if (visible && recipeId) loadData() }

    function loadData() {
        ingModel.clear()
        var data = recipeService.getById(recipeId)
        favIcon.text = recipeService.isFavorited(session.userId, recipeId) ? "favorite" : "favorite_border"
        nameLabel.text = data.name || ""
        // 标签
        var tagIds = data.tags || []
        var allTags = tagService.getAll()
        var tagNames = []
        for (var i = 0; i < tagIds.length; i++) {
            for (var j = 0; j < allTags.length; j++) {
                if (allTags[j].id === tagIds[i]) tagNames.push(allTags[j].name)
            }
        }
        tagLabel.text = tagNames.length > 0 ? tagNames.join(" · ") : "暂无标签"
        descLabel.text = data.description || ""
        timeLabel.text = "烹饪时间: " + (data.cookingTime || 0) + " min | " + (data.servings || 2) + " 人份"
        calLabel.text = "热量: " + (data.calPerServing || 0).toFixed(0) + " kcal/份 (总计 " + (data.totalCalories || 0).toFixed(0) + " kcal)"
        proLabel.text = "蛋白质: " + (data.proPerServing || 0).toFixed(1) + " g/份"
        carbLabel.text = "碳水: " + (data.carbPerServing || 0).toFixed(1) + " g/份"
        fatLabel.text = "脂肪: " + (data.fatPerServing || 0).toFixed(1) + " g/份"
        var ings = recipeService.getIngredients(recipeId)
        for (var i = 0; i < ings.length; i++) ingModel.append(ings[i])
    }

    Item {
        anchors { top: parent.top; topMargin: Theme.spacingSmall; left: parent.left; leftMargin: Theme.spacingSmall; right: parent.right; rightMargin: Theme.spacingSmall }
        height: 30
        Row { id: backRow; spacing: 4
            Icon { name: "arrow_back"; size: 24; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "返回"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
        }
        MouseArea {
            anchors { left: parent.left; verticalCenter: parent.verticalCenter }
            width: 80
            height: 30
            onClicked: navStack.pop()
        }

        Icon {
            id: favIcon
            name: "favorite_border"
            size: 28
            color: Theme.danger
            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
            MouseArea { anchors.fill: parent; onClicked: {
                recipeService.toggleFavorite(session.userId, recipeId)
                loadData()
            }}
        }
    }

    ListModel { id: ingModel }

    ScrollView {
        anchors { top: parent.top; topMargin: 40; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: btnBar.top; bottomMargin: Theme.spacingSmall }
        Column { spacing: Theme.spacingMedium
            Text { id: nameLabel; font.pixelSize: Theme.fontSizeLarge; color: Theme.textPrimary }
            Text { id: descLabel; font.pixelSize: Theme.fontSizeBody; color: Theme.textHint }
            Text { id: timeLabel; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
            Rectangle { width: parent.width; height: 1; color: Theme.borderLight }
            Text { text: "营养成分"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Text { id: calLabel; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Text { id: proLabel; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Text { id: carbLabel; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Text { id: fatLabel; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Rectangle { width: parent.width; height: 1; color: Theme.borderLight }
            Text { text: "标签"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Text {
                id: tagLabel
                font.pixelSize: Theme.fontSizeSmall; color: Theme.primary; wrapMode: Text.WordWrap; width: parent.width
            }
            Rectangle { width: parent.width; height: 1; color: Theme.borderLight }
            Text { text: "食材"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Repeater {
                model: ingModel
                delegate: Text { text: model.name+" · "+model.amount+model.unit; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            }
        }
    }

    // 底部按钮：编辑 / 删除
    Rectangle {
        id: btnBar
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
        height: 54
        color: Theme.bgCard
        Rectangle {
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 1
            color: Theme.borderLight
        }
        Row {
            anchors.centerIn: parent
            spacing: Theme.spacingMedium
            Rectangle {
                width: 120
                height: 40
                radius: Theme.radiusLarge
                color: Theme.primary
                Text { anchors.centerIn: parent; text: "编辑"; font.pixelSize: Theme.fontSizeBody; color: "white" }
                MouseArea { anchors.fill: parent; onClicked: {
                    var c = Qt.createComponent("qrc:/qml/RecipeEditPage.qml")
                    if (c.status === Component.Ready) navStack.push(c.createObject(navStack, { recipeId: root.recipeId }))
                }}
            }
            Rectangle {
                width: 120
                height: 40
                radius: Theme.radiusLarge
                color: Theme.danger
                Text { anchors.centerIn: parent; text: "删除"; font.pixelSize: Theme.fontSizeBody; color: "white" }
                MouseArea { anchors.fill: parent; onClicked: delDialog.open() }
            }
        }
    }

    Dialog {
        id: delDialog
        title: "删除菜谱"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        Text { text: "确认删除该菜谱？"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
        onAccepted: { recipeService.deleteRecipe(recipeId); navStack.pop() }
    }
}
