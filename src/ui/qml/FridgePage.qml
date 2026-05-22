import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "冰箱"
    property string familyId: ""

    background: Rectangle { color: Theme.bgPage }

    Component.onCompleted: { loadFridge() }
    onVisibleChanged: { if (visible) loadFridge() }

    function loadFridge() {
        var fams = familyService.getUserFamilies(session.userId)
        if (fams.length > 0) familyId = fams[0].familyId
        model.clear()
        var items = fridgeService.getStock(familyId)
        for (var i = 0; i < items.length; i++) model.append(items[i])
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
        text: "冰箱空空如也 — 去首页生成菜单\n再点采购清单一键入库"
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: Theme.fontSizeBody
        color: Theme.textHint
    }

    ListView {
        anchors { top: parent.top; topMargin: 50; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: parent.bottom; bottomMargin: 60 }
        spacing: 4
        model: model
        delegate: Rectangle {
            width: ListView.view.width
            height: 52
            radius: Theme.radiusSmall
            color: Theme.bgCard
            border { width: 1; color: Theme.borderLight }
            Row {
                anchors { left: parent.left; leftMargin: 12; verticalCenter: parent.verticalCenter }
                spacing: Theme.spacingMedium
                Text {
                    text: "●"
                    font.pixelSize: 10
                    color: {
                        switch(model.category) {
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
                Column { anchors.verticalCenter: parent.verticalCenter
                    Text { text: model.name; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
                    Text {
                        text: model.quantity.toFixed(0) + " " + model.unit + " · " + (model.expiryDate || "无过期日")
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.textHint
                    }
                }
            }
        }
    }

}
