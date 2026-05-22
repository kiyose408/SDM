import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "冰箱"
    property string familyId: ""
    property string filterCat: "all"
    property bool calibrateMode: false

    background: Rectangle { color: Theme.bgPage }

    Component.onCompleted: { loadFridge(); loadHistory() }
    onVisibleChanged: { if (visible) { loadFridge(); loadHistory() } }

    function initFamily() {
        var fams = familyService.getUserFamilies(session.userId)
        if (fams.length > 0) familyId = fams[0].familyId
    }

    function refreshInventory() {
        initFamily()
        invModel.clear()
        var items = fridgeService.getStock(familyId)
        for (var i = 0; i < items.length; i++) {
            var item = items[i]
            if (item.quantity <= 0) continue
            if (filterCat !== "all" && item.category !== filterCat) continue
            var days = 0
            if (item.purchaseDate) {
                var pd = new Date(item.purchaseDate)
                var now = new Date()
                days = Math.floor((now - pd) / 86400000)
            }
            item.ageDays = days
            invModel.append(item)
        }
    }
    function loadFridge() { refreshInventory() }

    function loadHistory() {
        initFamily()
        histModel.clear()
        var hist = consumptionService.getConsumptionHistory(familyId, 50)
        for (var i = 0; i < hist.length; i++) histModel.append(hist[i])
    }

    // 入库天数 → 颜色
    function ageColor(days) {
        if (days < 3) return "#64C288"
        if (days < 7) return "#F9D07B"
        return "#E86B6B"
    }
    function catColor(cat) {
        switch(cat) {
            case "meat":      return "#D32F2F"
            case "seafood":   return "#1976D2"
            case "vegetable": return "#388E3C"
            case "staple":    return "#F57C00"
            case "dairy":     return "#7B1FA2"
            case "condiment": return "#757575"
            default:          return "#999999"
        }
    }

    ListModel { id: invModel }
    ListModel { id: histModel }

    // ═══ 标题行 ═══
    Row {
        anchors { top: parent.top; topMargin: Theme.spacingMedium; left: parent.left; leftMargin: Theme.spacingMedium }
        spacing: Theme.spacingSmall
        Icon { name: "kitchen"; size: 24; color: Theme.primary; anchors.verticalCenter: parent.verticalCenter }
        Text { text: "冰箱"; font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
    }

    // ═══ Tab 切换 ═══
    Row {
        id: tabs
        anchors { top: parent.top; topMargin: 50; left: parent.left; leftMargin: Theme.spacingMedium }
        spacing: 16
        property int currentTab: 0

        Text { text: "📦 库存"; font.pixelSize: Theme.fontSizeBody; color: tabs.currentTab===0 ? Theme.primary : Theme.textHint; font.bold: tabs.currentTab===0
            MouseArea { anchors.fill: parent; onClicked: { tabs.currentTab = 0 } }
        }
        Text { text: "📋 历史"; font.pixelSize: Theme.fontSizeBody; color: tabs.currentTab===1 ? Theme.primary : Theme.textHint; font.bold: tabs.currentTab===1
            MouseArea { anchors.fill: parent; onClicked: { tabs.currentTab = 1 } }
        }
    }

    // ═══ 校准按钮（库存 Tab 右上角） ═══
    Rectangle {
        visible: tabs.currentTab === 0
        anchors { right: parent.right; rightMargin: Theme.spacingMedium; verticalCenter: tabs.verticalCenter }
        width: 66; height: 26; radius: 13
        color: calibrateMode ? Theme.danger : Theme.bgCard
        border { width: 1; color: Theme.borderLight }
        Text { anchors.centerIn: parent; text: calibrateMode ? "退出校准" : "校准"; font.pixelSize: Theme.fontSizeSmall; color: calibrateMode ? "white" : Theme.textPrimary }
        MouseArea { anchors.fill: parent; onClicked: { calibrateMode = !calibrateMode; refreshInventory() } }
    }

    // ═══ 筛选标签（库存 Tab） ═══
    Row {
        id: filterBar
        visible: tabs.currentTab === 0
        anchors { top: tabs.bottom; topMargin: 8; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium }
        spacing: 6
        Repeater {
            model: [{ cat: "all", label: "全部" },{ cat: "meat", label: "肉类" },{ cat: "seafood", label: "水产" },{ cat: "vegetable", label: "蔬菜" },{ cat: "staple", label: "主食" },{ cat: "dairy", label: "乳制品" },{ cat: "condiment", label: "调味料" }]
            delegate: Rectangle {
                width: 56; height: 28; radius: 14
                color: filterCat === modelData.cat ? Theme.primary : Theme.bgCard
                border { width: 1; color: Theme.borderLight }
                Text { anchors.centerIn: parent; text: modelData.label; font.pixelSize: Theme.fontSizeSmall; color: filterCat === modelData.cat ? "white" : Theme.textPrimary }
                MouseArea { anchors.fill: parent; onClicked: { filterCat = modelData.cat; refreshInventory() } }
            }
        }
    }

    Item {
        id: content
        anchors { top: filterBar.bottom; topMargin: 8; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: parent.bottom }

        // ═══ 库存网格 ═══
        GridView {
            visible: tabs.currentTab === 0
            anchors.fill: parent
            cellWidth: (width - 12) / 3
            cellHeight: calibrateMode ? 82 : 68
            model: invModel
            delegate: stockCard
        }

        // ═══ 消耗历史列表 ═══
        ListView {
            visible: tabs.currentTab === 1
            anchors.fill: parent
            spacing: 8
            model: histModel
            delegate: historyCard
            Text {
                anchors.centerIn: parent
                visible: histModel.count === 0
                text: "暂无消耗记录"
                font.pixelSize: Theme.fontSizeBody
                color: Theme.textHint
            }
        }
    }

    // ═══ 库存卡片 ═══
    Component {
        id: stockCard
        Rectangle {
            width: GridView.view.cellWidth - 4
            height: GridView.view.cellHeight - 8
            radius: Theme.radiusSmall
            color: Theme.bgCard
            border { width: 1; color: Theme.borderLight }

            Rectangle {
                anchors { top: parent.top; left: parent.left; right: parent.right }
                height: 3; radius: Theme.radiusSmall
                color: ageColor(model.ageDays || 0)
            }

            Column {
                anchors { fill: parent; topMargin: 7; leftMargin: 8; rightMargin: 4 }
                spacing: 1

                Row { spacing: 4
                    Rectangle { width: 8; height: 8; radius: 4; color: catColor(model.category); anchors.verticalCenter: parent.verticalCenter }
                    Text { text: model.name; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; elide: Text.ElideRight; width: parent.width - 16 }
                }
                Text { text: model.quantity.toFixed(0) + " " + model.unit; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; font.bold: true }
                Text { text: model.ageDays > 0 ? "入库 " + model.ageDays + " 天" : "今日入库"; font.pixelSize: Theme.fontSizeSmall; color: ageColor(model.ageDays || 0) }
            }

            MouseArea {
                anchors.fill: parent
                visible: calibrateMode
                onClicked: {
                    calPopup.ingId = model.ingredientId
                    calPopup.ingName = model.name
                    calPopup.curQty = model.quantity
                    calPopup.open()
                }
            }
        }
    }

    // ═══ 历史卡片 ═══
    Component {
        id: historyCard
        Rectangle {
            width: ListView.view.width
            height: 52
            radius: Theme.radiusSmall
            color: Theme.bgCard
            border { width: 1; color: Theme.borderLight }
            Row {
                anchors { left: parent.left; leftMargin: 12; verticalCenter: parent.verticalCenter }
                spacing: 8
                Text { text: "🍳"; font.pixelSize: 18; anchors.verticalCenter: parent.verticalCenter }
                Column { anchors.verticalCenter: parent.verticalCenter
                    Text { text: model.recipeName || ""; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
                    Text { text: (model.details || "") + " · " + model.createdAt.split("T")[0]; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
                }
            }
            // 撤销按钮
            Rectangle {
                width: 48; height: 26; radius: 13; color: Theme.bgCard; border { width: 1; color: Theme.danger }
                anchors { right: parent.right; rightMargin: 8; verticalCenter: parent.verticalCenter }
                Text { anchors.centerIn: parent; text: "撤销"; font.pixelSize: Theme.fontSizeSmall; color: Theme.danger }
                MouseArea { anchors.fill: parent; onClicked: {
                    consumptionService.undoConsume(model.menuItemId, session.userId)
                    loadFridge()
                    loadHistory()
                }}
            }
        }
    }

    // ═══ 校准弹窗 ═══
    Popup {
        id: calPopup
        width: parent.width - 40
        height: 180
        x: 20; y: parent.height / 2 - 90
        modal: true
        property string ingId: ""
        property string ingName: ""
        property double curQty: 0

        background: Rectangle { color: Theme.bgCard; radius: 16; border { width: 1; color: Theme.borderLight } }
        Column {
            anchors { fill: parent; topMargin: 16; leftMargin: 16; rightMargin: 16 }
            spacing: 12
            Text { text: "调整库存: " + calPopup.ingName; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Text { text: "当前: " + calPopup.curQty.toFixed(0) + "g"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
            Row { spacing: 8
                Text { text: "新数量:"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 120; height: 36; radius: Theme.radiusSmall; color: Theme.bgPage; border { width: 1; color: Theme.borderLight }
                    TextInput { id: newQtyInput; anchors { left: parent.left; leftMargin: 8; verticalCenter: parent.verticalCenter } font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; text: calPopup.curQty.toFixed(0) }
                }
            }
            Rectangle { width: parent.width; height: 36; radius: Theme.radiusLarge; color: Theme.primary
                Text { anchors.centerIn: parent; text: "确认更新"; font.pixelSize: Theme.fontSizeBody; color: "white" }
                MouseArea { anchors.fill: parent; onClicked: {
                    var newQty = parseFloat(newQtyInput.text) || calPopup.curQty
                    fridgeService.calibrateStock(familyId, calPopup.ingId, newQty, session.userId)
                    calPopup.close()
                    refreshInventory()
                }}
            }
        }
    }
}
