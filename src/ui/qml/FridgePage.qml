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

    Component.onCompleted: { loadFridge() }
    onVisibleChanged: { if (visible) loadFridge() }

    function refresh() {
        var fams = familyService.getUserFamilies(session.userId)
        if (fams.length > 0) familyId = fams[0].familyId
        model.clear()
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
            model.append(item)
        }
    }

    function loadFridge() { refresh() }

    // 入库天数 → 颜色
    function ageColor(days) {
        if (days < 3) return "#64C288"     // 新鲜绿
        if (days < 7) return "#F9D07B"     // 注意黄
        return "#E86B6B"                   // 警惕红
    }

    ListModel { id: model }

    Item {
        id: titleBar
        anchors { top: parent.top; topMargin: Theme.spacingMedium; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium }
        height: 28

        Row {
            anchors { left: parent.left; verticalCenter: parent.verticalCenter }
            spacing: Theme.spacingSmall
            Icon { name: "kitchen"; size: 24; color: Theme.primary; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "冰箱 · " + model.count + " 项"; font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
        }

        Rectangle {
            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
            width: 66; height: 28; radius: 14
            color: calibrateMode ? Theme.danger : Theme.bgCard
            border { width: 1; color: calibrateMode ? Theme.danger : Theme.borderLight }
            Text { anchors.centerIn: parent; text: calibrateMode ? "退出" : "校准"; font.pixelSize: Theme.fontSizeSmall; color: calibrateMode ? "white" : Theme.textPrimary }
            MouseArea { anchors.fill: parent; onClicked: calibrateMode = !calibrateMode }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: model.count === 0
        text: "冰箱空空如也\n去首页生成菜单 → 采购清单 → 一键入库"
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: Theme.fontSizeBody
        color: Theme.textHint
    }

    // 筛选标签栏
    Row {
        id: filterBar
        anchors { top: titleBar.bottom; topMargin: 8; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium }
        spacing: 6
        Repeater {
            model: [{ cat: "all", label: "全部" },{ cat: "meat", label: "肉类" },{ cat: "seafood", label: "水产" },{ cat: "vegetable", label: "蔬菜" },{ cat: "staple", label: "主食" },{ cat: "dairy", label: "乳制品" },{ cat: "condiment", label: "调味料" }]
            delegate: Rectangle {
                width: 56; height: 28; radius: 14
                color: filterCat === modelData.cat ? Theme.primary : Theme.bgCard
                border { width: 1; color: Theme.borderLight }
                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    font.pixelSize: Theme.fontSizeSmall
                    color: filterCat === modelData.cat ? "white" : Theme.textPrimary
                }
                MouseArea { anchors.fill: parent; onClicked: { filterCat = modelData.cat; refresh() } }
            }
        }
    }

    GridView {
        id: grid
        anchors { top: filterBar.bottom; topMargin: 8; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: parent.bottom }
        cellWidth: (grid.width - Theme.spacingSmall * 2) / 3
        cellHeight: 76
        model: model
        delegate: Rectangle {
            width: grid.cellWidth - 4
            height: 68
            radius: Theme.radiusSmall
            color: Theme.bgCard
            border { width: 1; color: calibrateMode ? Theme.danger : Theme.borderLight }

            Rectangle {
                anchors { top: parent.top; left: parent.left; right: parent.right }
                height: 3
                radius: Theme.radiusSmall
                color: ageColor(model.ageDays || 0)
            }

            Column {
                anchors { fill: parent; topMargin: 7; leftMargin: 8; rightMargin: 4 }
                spacing: 1

                Row { spacing: 4
                    Rectangle { width: 8; height: 8; radius: 4; color: catColor(model.category); anchors.verticalCenter: parent.verticalCenter }
                    Text {
                        text: model.name
                        font.pixelSize: Theme.fontSizeBody
                        color: Theme.textPrimary
                        elide: Text.ElideRight
                        width: grid.cellWidth - 24
                    }
                }

                Text {
                    text: model.quantity.toFixed(0) + " " + model.unit
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    font.bold: true
                }

                Text {
                    text: model.ageDays > 0 ? "入库 " + model.ageDays + " 天" : "今日入库"
                    font.pixelSize: Theme.fontSizeSmall
                    color: ageColor(model.ageDays || 0)
                }
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

    // ═══ 校准弹窗 ═══
    Popup {
        id: calPopup
        width: Math.min(parent.width - 40, 320)
        height: 170
        x: (parent.width - width) / 2; y: parent.height / 2 - 85
        modal: true; closePolicy: Popup.CloseOnEscape
        property string ingId: ""
        property string ingName: ""
        property double curQty: 0

        background: Rectangle { color: Theme.bgCard; radius: Theme.radiusMedium; border { width: 1; color: Theme.borderLight } }
        Column {
            anchors { fill: parent; topMargin: 16; leftMargin: 16; rightMargin: 16 }
            spacing: 12
            Text { text: calPopup.ingName; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; font.bold: true }
            Text { text: "当前库存: " + calPopup.curQty.toFixed(0) + "g"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
            Row { spacing: 8
                Text { text: "新数量:"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                Rectangle { width: 120; height: 36; radius: Theme.radiusSmall; color: Theme.bgPage; border { width: 1; color: Theme.borderLight }
                    TextInput {
                        id: newQtyInput; anchors { left: parent.left; leftMargin: 8; verticalCenter: parent.verticalCenter }
                        font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary
                        text: calPopup.curQty.toFixed(0)
                    }
                }
                Text { text: "g"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint; anchors.verticalCenter: parent.verticalCenter }
            }
            Row {
                width: parent.width; spacing: 8
                Rectangle { width: (parent.width - 8) / 2; height: 36; radius: Theme.radiusLarge; color: Theme.danger
                    Text { anchors.centerIn: parent; text: "清空"; font.pixelSize: Theme.fontSizeBody; color: "white" }
                    MouseArea { anchors.fill: parent; onClicked: confirmClear.open() }
                }
                Rectangle { width: (parent.width - 8) / 2; height: 36; radius: Theme.radiusLarge; color: Theme.primary
                    Text { anchors.centerIn: parent; text: "确认更新"; font.pixelSize: Theme.fontSizeBody; color: "white" }
                    MouseArea { anchors.fill: parent; onClicked: {
                        var n = parseFloat(newQtyInput.text)
                        if (isNaN(n) || n < 0) n = 0
                        fridgeService.calibrateStock(familyId, calPopup.ingId, n, session.userId)
                        calPopup.close()
                        refresh()
                    }}
                }
            }
            // 清空确认弹窗
            Popup {
                id: confirmClear
                width: 260; height: 120
                x: (parent.width - width) / 2 > 0 ? (parent.width - width) / 2 : 10
                y: (parent.height - height) / 2 > 0 ? (parent.height - height) / 2 : 10
                modal: true; closePolicy: Popup.CloseOnEscape
                background: Rectangle { color: Theme.bgCard; radius: Theme.radiusMedium; border { width: 2; color: Theme.danger } }
                contentItem: Column {
                    anchors.centerIn: parent
                    spacing: 14
                    Text {
                        text: "确认清空 \"" + calPopup.ingName + "\"？"
                        font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary
                    }
                    Row {
                        spacing: 12
                        Rectangle { width: 100; height: 34; radius: Theme.radiusSmall; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                            Text { anchors.centerIn: parent; text: "取消"; font.pixelSize: Theme.fontSizeBody; color: Theme.textHint }
                            MouseArea { anchors.fill: parent; onClicked: confirmClear.close() }
                        }
                        Rectangle { width: 100; height: 34; radius: Theme.radiusSmall; color: Theme.danger
                            Text { anchors.centerIn: parent; text: "确认清空"; font.pixelSize: Theme.fontSizeBody; color: "white" }
                            MouseArea { anchors.fill: parent; onClicked: {
                                fridgeService.calibrateStock(familyId, calPopup.ingId, 0, session.userId)
                                confirmClear.close()
                                calPopup.close()
                                refresh()
                            }}
                        }
                    }
                }
            }
        }
    }
}
