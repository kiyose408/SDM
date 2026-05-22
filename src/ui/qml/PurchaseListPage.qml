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

    Row {
        anchors { top: parent.top; topMargin: Theme.spacingMedium; left: parent.left; leftMargin: Theme.spacingMedium }
        spacing: Theme.spacingSmall
        Icon { name: "shopping_basket"; size: 24; color: Theme.primary; anchors.verticalCenter: parent.verticalCenter }
        Text { text: "今日采购 · " + today; font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
    }

    Text {
        anchors.centerIn: parent
        visible: model.count === 0
        text: "无需采购 — 冰箱库存充足"
        font.pixelSize: Theme.fontSizeBody
        color: Theme.textHint
    }

    ListView {
        anchors { top: parent.top; topMargin: 50; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: parent.bottom; bottomMargin: 60 }
        spacing: 4
        model: model
        delegate: Rectangle {
            width: ListView.view.width
            height: 44
            radius: Theme.radiusSmall
            color: Theme.bgCard
            border { width: 1; color: Theme.borderLight }
            Row {
                anchors { left: parent.left; leftMargin: 12; verticalCenter: parent.verticalCenter }
                spacing: Theme.spacingSmall
                Text {
                    text: "●"; font.pixelSize: 8
                    color: {
                        switch(model.cat) {
                            case "meat":      return "#D32F2F"
                            case "seafood":   return "#1976D2"
                            case "vegetable": return "#388E3C"
                            case "staple":    return "#F57C00"
                            case "dairy":     return "#7B1FA2"
                            case "condiment": return "#757575"
                            default:          return "#999999"
                        }
                    }
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: model.name + "  需 " + model.need.toFixed(0) + "g"
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            Row {
                anchors { right: parent.right; rightMargin: 8; verticalCenter: parent.verticalCenter }
                spacing: 2
                Rectangle { width: 20; height: 20; radius: 10; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "−"; anchors.centerIn: parent; font.pixelSize: 12; color: Theme.textHint }
                    MouseArea { anchors.fill: parent; onClicked: {
                        model.bought = Math.max(0, (model.bought || 0) - 50)
                    }}
                }
                Text {
                    text: (model.bought || 0).toFixed(0) + "g"
                    font.pixelSize: Theme.fontSizeSmall
                    color: (model.bought || 0) >= model.need ? Theme.primary : Theme.secondary
                    anchors.verticalCenter: parent.verticalCenter
                }
                Rectangle { width: 20; height: 20; radius: 10; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { text: "+"; anchors.centerIn: parent; font.pixelSize: 12; color: Theme.textHint }
                    MouseArea { anchors.fill: parent; onClicked: {
                        model.bought = (model.bought || 0) + 50
                    }}
                }
            }
        }
    }

    Rectangle {
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right; bottomMargin: 10 }
        height: 44
        radius: Theme.radiusLarge
        color: model.count > 0 ? Theme.primary : Theme.textHint
        Text { anchors.centerIn: parent; text: "确认采购 · 一键入库"; font.pixelSize: Theme.fontSizeBody; color: "white" }
        MouseArea { anchors.fill: parent; onClicked: {
            if (model.count === 0) return
            var items = []
            for (var i = 0; i < model.count; i++) {
                items.push({ ingredientId: model.get(i).ingredientId, bought: model.get(i).bought || 0 })
            }
            purchaseService.confirmPurchase(familyId, session.userId, items)
            navStack.pop()
        } }
    }
}
