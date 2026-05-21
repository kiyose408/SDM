import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root; title: "菜谱列表"
    background: Rectangle { color: Theme.bgPage }

    property string activeTag: "all"
    property var allTags: []

    Component.onCompleted: { allTags = tagService.getAll(); loadTags() }

    Item {
        anchors { top: parent.top; topMargin: Theme.spacingSmall; left: parent.left; leftMargin: Theme.spacingSmall }
        width: backRow.width; height: backRow.height
        Row { id: backRow; spacing: 4
            Icon { name: "arrow_back"; size: 24; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "返回"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
        }
        MouseArea { anchors.fill: parent; onClicked: navStack.pop() }
    }

    Rectangle {
        anchors { top: parent.top; topMargin: 40; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium }
        height: 40; radius: Theme.radiusLarge; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
        Row {
            anchors { left: parent.left; leftMargin: Theme.spacingSmall; verticalCenter: parent.verticalCenter }
            spacing: Theme.spacingSmall
            Icon { name: "search"; size: 20; color: Theme.textHint; anchors.verticalCenter: parent.verticalCenter }
            TextInput {
                id: searchInput; width: parent.parent.width - 80; anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary
                Text { text: "搜索菜谱..."; font: parent.font; color: Theme.textHint; visible: !searchInput.text && !searchInput.activeFocus }
                onTextChanged: refresh()
            }
        }
    }

    ScrollView {
        anchors { top: parent.top; topMargin: 86; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium }
        height: 44; contentWidth: tagRow.width
        Row {
            id: tagRow; spacing: Theme.spacingSmall
            Rectangle {
                width: 44; height: 28; radius: Theme.radiusSmall
                color: activeTag === "all" ? Theme.primary : Theme.bgCard
                border { width: 1; color: activeTag === "all" ? Theme.primary : Theme.borderLight }
                Text { anchors.centerIn: parent; text: "全部"; font.pixelSize: Theme.fontSizeSmall; color: activeTag === "all" ? "white" : Theme.textPrimary }
                MouseArea { anchors.fill: parent; onClicked: { activeTag = "all"; refresh() } }
            }
            Repeater {
                model: allTags
                delegate: Rectangle {
                    width: modelData.name.length * 14 + 16; height: 28; radius: Theme.radiusSmall
                    color: activeTag === modelData.id ? Theme.primary : Theme.bgCard
                    border { width: 1; color: activeTag === modelData.id ? Theme.primary : Theme.borderLight }
                    Text { anchors.centerIn: parent; text: modelData.name; font.pixelSize: Theme.fontSizeSmall; color: activeTag === modelData.id ? "white" : Theme.textPrimary }
                    MouseArea { anchors.fill: parent; onClicked: { activeTag = activeTag === modelData.id ? "all" : modelData.id; refresh() } }
                }
            }
        }
    }

    ListView {
        anchors { top: parent.top; topMargin: 136; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: parent.bottom }
        spacing: Theme.spacingSmall
        model: ListModel { id: model }
        delegate: Rectangle {
            width: ListView.view.width; height: 56; radius: Theme.radiusSmall
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

    function loadTags() { allTags = tagService.getAll() }

    function refresh() {
        model.clear()
        var items = recipeService.getAll()
        for (var i = 0; i < items.length; i++) {
            var item = items[i]
            var kw = searchInput.text.trim()
            if (kw && item.name.indexOf(kw) < 0) continue
            // 标签筛选
            if (activeTag !== "all") {
                var tags = tagService.getTagIdsByRecipe(item.id)
                var found = false
                for (var j = 0; j < tags.length; j++) { if (tags[j] === activeTag) { found = true; break } }
                if (!found) continue
            }
            model.append(item)
        }
    }

}
