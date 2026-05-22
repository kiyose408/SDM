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

    GridView {
        id: grid
        anchors { top: parent.top; topMargin: 50; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: parent.bottom; bottomMargin: 56 }
        cellWidth: (grid.width - 8) / 2
        cellHeight: 90
        model: model
        delegate: Rectangle {
            width: grid.cellWidth - 4
            height: 82
            radius: Theme.radiusSmall
            color: Theme.bgCard
            border { width: 1; color: Theme.borderLight }

            // 类别色条
            Rectangle {
                anchors { top: parent.top; left: parent.left; right: parent.right }
                height: 3; radius: Theme.radiusSmall
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
            }

            Column {
                anchors { fill: parent; topMargin: 8; leftMargin: 8; rightMargin: 6 }
                spacing: 2

                Text {
                    text: model.name
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    elide: Text.ElideRight
                    width: parent.width
                }

                Text {
                    text: "需 " + model.need.toFixed(0) + "g"
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.textHint
                }

                Row {
                    spacing: 2
                    Rectangle { width: 22; height: 22; radius: 11; color: Theme.bgPage; border { width: 1; color: Theme.borderLight }
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
                        width: 36; horizontalAlignment: Text.AlignHCenter
                    }
                    Rectangle { width: 22; height: 22; radius: 11; color: Theme.bgPage; border { width: 1; color: Theme.borderLight }
                        Text { text: "+"; anchors.centerIn: parent; font.pixelSize: 12; color: Theme.textHint }
                        MouseArea { anchors.fill: parent; onClicked: {
                            model.bought = (model.bought || 0) + 50
                        }}
                    }
                    // 一键足量
                    Rectangle { width: 36; height: 22; radius: 6; color: (model.bought || 0) >= model.need ? Theme.primary : Theme.info
                        Text { anchors.centerIn: parent; text: (model.bought || 0) >= model.need ? "✓" : "足量"; font.pixelSize: 10; color: "white" }
                        MouseArea { anchors.fill: parent; onClicked: { model.bought = model.need } }
                    }
                }
            }
        }
    }

    Row {
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right; bottomMargin: 8; margins: Theme.spacingMedium }
        spacing: 8

        Rectangle {
            width: 80; height: 44
            radius: Theme.radiusLarge
            color: Theme.bgCard
            border { width: 1; color: Theme.borderLight }
            Text { anchors.centerIn: parent; text: "返回"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            MouseArea { anchors.fill: parent; onClicked: navStack.pop() }
        }

        Rectangle {
            width: parent.width - 88; height: 44
            radius: Theme.radiusLarge
            color: model.count > 0 ? Theme.primary : Theme.textHint
            Text { anchors.centerIn: parent; text: "确认采购 · 一键入库"; font.pixelSize: Theme.fontSizeBody; color: "white" }
            MouseArea { anchors.fill: parent; onClicked: {
                if (model.count === 0) return
                receiptModel.clear()
                for (var i = 0; i < model.count; i++) {
                    var item = model.get(i)
                    if ((item.bought || 0) > 0)
                        receiptModel.append({ name: item.name, qty: (item.bought || 0).toFixed(0) })
                }
                if (receiptModel.count === 0) { navStack.pop(); return }
                receiptPopup.open()
            }}
        }
    }

    ListModel { id: receiptModel }

    // ═══ 小票确认弹窗 ═══
    Popup {
        id: receiptPopup
        width: Math.min(parent.width - 40, 280)
        height: Math.min(receiptModel.count * 28 + 120, parent.height * 0.7)
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        modal: true; closePolicy: Popup.CloseOnEscape
        padding: 0

        background: Rectangle { color: "#FFFDF5"; radius: Theme.radiusMedium; border { width: 1; color: Theme.borderLight } }

        contentItem: Item {
            Column {
                id: col
                anchors { fill: parent; topMargin: 12; leftMargin: 12; rightMargin: 12; bottomMargin: 8 }
                spacing: 4

                Text {
                    text: "📋 采购小票"
                    font.pixelSize: Theme.fontSizeTitle; font.bold: true
                    color: Theme.textPrimary
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Rectangle { width: parent.width; height: 1; color: Theme.textHint; opacity: 0.3 }

                ListView {
                    width: parent.width
                    height: Math.min(receiptModel.count * 28, 220)
                    model: receiptModel
                    clip: true
                    delegate: Row {
                        width: parent.width; height: 24
                        Text {
                            text: model.name + "  ··························································"
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                            width: parent.width - 48
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        Text {
                            text: model.qty + "g"
                            font.pixelSize: Theme.fontSizeBody
                            color: Theme.textPrimary
                            width: 48
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                Rectangle { width: parent.width; height: 1; color: Theme.textHint; opacity: 0.3 }
                Item { height: 4; width: 1 }
            }

            // 按钮固定在底部
            Row {
                anchors { bottom: parent.bottom; bottomMargin: 8; horizontalCenter: parent.horizontalCenter }
                spacing: 12
                Rectangle { width: 80; height: 34; radius: Theme.radiusSmall; color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                    Text { anchors.centerIn: parent; text: "取消"; font.pixelSize: Theme.fontSizeBody; color: Theme.textHint }
                    MouseArea { anchors.fill: parent; onClicked: receiptPopup.close() }
                }
                Rectangle { width: 80; height: 34; radius: Theme.radiusSmall; color: Theme.primary
                    Text { anchors.centerIn: parent; text: "确认采购"; font.pixelSize: Theme.fontSizeBody; color: "white" }
                    MouseArea { anchors.fill: parent; onClicked: {
                        var items = []
                        for (var i = 0; i < model.count; i++) {
                            var item = model.get(i)
                            if ((item.bought || 0) > 0)
                                items.push({ ingredientId: item.ingredientId, bought: item.bought })
                        }
                        purchaseService.confirmPurchase(familyId, session.userId, items)
                        receiptPopup.close()
                        navStack.pop()
                    }}
                }
            }
        }
    }
}
