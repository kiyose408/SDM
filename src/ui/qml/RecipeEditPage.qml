import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root; title: ""
    property string recipeId: ""    // 空=新建，非空=编辑
    property string familyId: ""

    background: Rectangle { color: Theme.bgPage }

    Component.onCompleted: {
        allTags = tagService.getAll()
        loadIngredients()
        if (recipeId) {
            loadExistingTags()
            var d = recipeService.getById(recipeId)
            nameInput.text    = d.name || ""
            descInput.text    = d.description || ""
            timeInput.text    = d.cookingTime ? String(d.cookingTime) : ""
            servingsInput.text = d.servings ? String(d.servings) : "2"
            for (var i = 0; i < mtModel.count; i++) { if (mtModel.get(i).value === d.mealType) { mtBox.currentIndex = i } }
            // 回填已有食材
            var existing = recipeService.getIngredients(recipeId)
            for (var j = 0; j < existing.length; j++) {
                var ex = existing[j]
                for (var k = 0; k < ingModel.count; k++) {
                    if (ingModel.get(k).name === ex.name) {
                        var row = ingRepeater.itemAt(k)
                        if (row) { row.isChecked = true; row.amount = String(ex.amount) }
                        break
                    }
                }
            }
        }
    }

    function loadIngredients() {
        ingModel.clear()
        var all = ingredientService.getAll()
        for (var i = 0; i < all.length; i++) ingModel.append(all[i])
    }

    ListModel { id: mtModel
        ListElement { text: "不限";   value: "any" }
        ListElement { text: "早餐";   value: "breakfast" }
        ListElement { text: "午餐";   value: "lunch" }
        ListElement { text: "晚餐";   value: "dinner" }
    }
    ListModel { id: ingModel }
    property var selectedTags: []
    property var allTags: []

    function loadExistingTags() {
        selectedTags = []
        var ids = tagService.getTagIdsByRecipe(recipeId)
        for (var i = 0; i < ids.length; i++) selectedTags.push(ids[i])
    }

    // 返回
    Item {
        anchors { top: parent.top; topMargin: Theme.spacingSmall; left: parent.left; leftMargin: Theme.spacingSmall }
        width: backRow.width; height: backRow.height
        Row { id: backRow; spacing: 4
            Icon { name: "arrow_back"; size: 24; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "返回"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
        }
        MouseArea { anchors.fill: parent; onClicked: navStack.pop() }
    }

    ScrollView {
        anchors { top: parent.top; topMargin: 40; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: saveBtn.top; bottomMargin: Theme.spacingSmall }
        Column {
            spacing: Theme.spacingMedium
            width: parent.width
            Text { text: recipeId ? "编辑菜谱" : "创建菜谱"; font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary }

            // ---- 菜名 ----
            Item {
                width: parent.width
                height: 44
                Rectangle {
                    anchors.fill: parent
                    radius: Theme.radiusLarge
                    color: Theme.bgCard
                    border { width: 1; color: Theme.borderLight }
                }
                TextInput {
                    id: nameInput
                    anchors { left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    Text { text: "菜名"; font: parent.font; color: Theme.textHint; visible: !nameInput.text && !nameInput.activeFocus }
                }
            }
            // ---- 简介 ----
            Item {
                width: parent.width
                height: 44
                Rectangle {
                    anchors.fill: parent
                    radius: Theme.radiusLarge
                    color: Theme.bgCard
                    border { width: 1; color: Theme.borderLight }
                }
                TextInput {
                    id: descInput
                    anchors { left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    Text { text: "简介"; font: parent.font; color: Theme.textHint; visible: !descInput.text && !descInput.activeFocus }
                }
            }
            // ---- 时间 + 份数 ----
            Row {
                width: parent.width; spacing: Theme.spacingSmall
                Item {
                    width: (parent.width - Theme.spacingSmall) / 2; height: 44
                    Rectangle { anchors.fill: parent; radius: Theme.radiusSmall; color: Theme.bgCard; border { width: 1; color: Theme.borderLight } }
                    TextInput { id: timeInput; anchors.centerIn: parent; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; horizontalAlignment: TextInput.AlignHCenter
                        Text { text: "时间(min)"; font: parent.font; color: Theme.textHint; visible: !timeInput.text && !timeInput.activeFocus; anchors.centerIn: parent }
                    }
                }
                Item {
                    width: (parent.width - Theme.spacingSmall) / 2; height: 44
                    Rectangle { anchors.fill: parent; radius: Theme.radiusSmall; color: Theme.bgCard; border { width: 1; color: Theme.borderLight } }
                    TextInput { id: servingsInput; anchors.centerIn: parent; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; horizontalAlignment: TextInput.AlignHCenter; text: "2"
                        Text { text: "份数"; font: parent.font; color: Theme.textHint; visible: !servingsInput.text && !servingsInput.activeFocus; anchors.centerIn: parent }
                    }
                }
            }
            Text { text: "餐别"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            ComboBox { id: mtBox; model: mtModel; textRole: "text"; width: parent.width }

            Rectangle { width: parent.width; height: 1; color: Theme.borderLight }

            Text { text: "食材清单"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Repeater {
                id: ingRepeater
                model: ingModel
                delegate: Row {
                    spacing: Theme.spacingSmall
                    property alias isChecked: cb.checked
                    property alias amount: amt.text
                    CheckBox { id: cb; checked: false }
                    Text {
                        text: model.name + " (" + model.caloriesPer100 + " kcal/100" + model.unit + ")"
                        font.pixelSize: Theme.fontSizeBody
                        color: Theme.textPrimary
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Rectangle {
                        width: 60; height: 28; radius: Theme.radiusSmall; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                        TextInput { id: amt; anchors.centerIn: parent; font.pixelSize: Theme.fontSizeBody; text: "0"; validator: DoubleValidator {} }
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            // ---- 标签选择 ----
            Rectangle { width: parent.width; height: 1; color: Theme.borderLight }
            Text { text: "标签"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }

            Flow {
                width: parent.width; spacing: Theme.spacingSmall
                Repeater {
                    model: allTags
                    delegate: Rectangle {
                        width: 72; height: 28; radius: Theme.radiusSmall
                        color: selectedTags.indexOf(modelData.id) >= 0 ? Theme.primary : Theme.bgCard
                        border { width: 1; color: selectedTags.indexOf(modelData.id) >= 0 ? Theme.primary : Theme.borderLight }
                        Text { anchors.centerIn: parent; text: modelData.name; font.pixelSize: Theme.fontSizeSmall; color: selectedTags.indexOf(modelData.id) >= 0 ? "white" : Theme.textPrimary }
                        MouseArea { anchors.fill: parent; onClicked: {
                            var idx = selectedTags.indexOf(modelData.id)
                            if (idx >= 0) selectedTags.splice(idx, 1)
                            else selectedTags.push(modelData.id)
                            selectedTags = selectedTags  // 触发绑定刷新
                        }}
                    }
                }
            }
        }
    }

    // 保存按钮
    Rectangle {
        id: saveBtn
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
        height: 54; color: Theme.bgCard
        Rectangle { anchors { top: parent.top; left: parent.left; right: parent.right } height: 1; color: Theme.borderLight }
        Rectangle {
            anchors.centerIn: parent; width: parent.width - Theme.spacingMedium * 2; height: 40; radius: Theme.radiusLarge; color: Theme.primary
            Text { anchors.centerIn: parent; text: "保存"; font.pixelSize: Theme.fontSizeBody; color: "white" }
            MouseArea { anchors.fill: parent; onClicked: saveRecipe() }
        }
    }

    function saveRecipe() {
        var data = {
            name: nameInput.text, description: descInput.text,
            cookingTime: parseInt(timeInput.text) || 0,
            servings: parseInt(servingsInput.text) || 2,
            mealType: mtModel.get(mtBox.currentIndex).value
        }
        var ings = []
        for (var i = 0; i < ingModel.count; i++) {
            var row = ingRepeater.itemAt(i)
            if (row && row.isChecked && parseFloat(row.amount) > 0) {
                ings.push({ ingredientId: ingModel.get(i).id, amount: parseFloat(row.amount) })
            }
        }
        if (recipeId) {
            recipeService.updateRecipe(recipeId, data, ings)
            tagService.setRecipeTags(recipeId, selectedTags)
        } else {
            var r = recipeService.createRecipe(data, ings, session.userId, familyId)
            if (r.success) tagService.setRecipeTags(r.recipeId, selectedTags)
        }
        navStack.pop()
    }

}
