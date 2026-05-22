import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "冰箱"
    property string familyId: ""
    property string filterCat: "all"

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

    Row {
        anchors { top: parent.top; topMargin: Theme.spacingMedium; left: parent.left; leftMargin: Theme.spacingMedium }
        spacing: Theme.spacingSmall
        Icon { name: "kitchen"; size: 24; color: Theme.primary; anchors.verticalCenter: parent.verticalCenter }
        Text { text: "冰箱 · " + model.count + " 项"; font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
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
        anchors { top: parent.top; topMargin: 50; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium }
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
            border { width: 1; color: Theme.borderLight }

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
}
