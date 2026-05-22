import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "采购清单"

    property string familyId: ""
    property string today: ""

    background: Rectangle { color: Theme.bgPage }

    Component.onCompleted: {
        var d = new Date()
        today = d.getFullYear() + "-" + String(d.getMonth()+1).padStart(2,'0') + "-" + String(d.getDate()).padStart(2,'0')
        var fams = familyService.getUserFamilies(session.userId)
        if (fams.length > 0) familyId = fams[0].familyId
        refresh()
    }

    function refresh() {
        model.clear()
        var items = purchaseService.getTodayPurchaseList(familyId, today)
        for (var i = 0; i < items.length; i++) model.append(items[i])
    }

    ListModel { id: model }

    Column {
        anchors { fill: parent; topMargin: Theme.spacingMedium; leftMargin: Theme.spacingMedium; rightMargin: Theme.spacingMedium }
        spacing: Theme.spacingMedium

        Row {
            spacing: Theme.spacingSmall
            Icon { name: "shopping_basket"; size: 24; color: Theme.primary; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "今日采购清单 · " + today; font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
        }

        Repeater {
            model: ListModel { id: grouped; Component.onCompleted: {} }
        }

        ListView {
            width: parent.width
            height: parent.height - 60
            spacing: Theme.spacingSmall
            model: model
            delegate: Rectangle {
                width: parent.width
                height: 44
                radius: Theme.radiusSmall
                color: Theme.bgCard
                border { width: 1; color: Theme.borderLight }
                Row {
                    anchors { left: parent.left; leftMargin: 12; verticalCenter: parent.verticalCenter }
                    spacing: Theme.spacingSmall
                    Text { text: "●"; font.pixelSize: 8; color: catColor(model.cat); anchors.verticalCenter: parent.verticalCenter }
                    Text { text: model.name; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                    Text { text: model.amount; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint; anchors.verticalCenter: parent.verticalCenter }
                }
            }
        }

        Text {
            visible: model.count === 0
            text: "暂无菜单 — 请先生成"
            font.pixelSize: Theme.fontSizeBody
            color: Theme.textHint
            anchors.centerIn: parent
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
