import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "家庭成员"

    property string familyId: ""
    property string currentUserId: ""

    background: Rectangle { color: Theme.bgPage }

    // 返回按钮
    Item {
        anchors { top: parent.top; topMargin: Theme.spacingSmall; left: parent.left; leftMargin: Theme.spacingSmall }
        width: backRow.width; height: backRow.height
        Row { id: backRow; spacing: 4
            Icon { name: "arrow_back"; size: 24; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "返回"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
        }
        MouseArea { anchors.fill: parent; onClicked: navStack.pop() }
    }

    ListView {
        anchors { top: parent.top; topMargin: 40 }
        id: memberList
        anchors { fill: parent; margins: Theme.spacingMedium }
        spacing: Theme.spacingSmall
        model: ListModel { id: memberModel }

        header: Text {
            text: "家庭成员 (" + memberModel.count + ")"
            font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary
        }

        delegate: Rectangle {
            width: parent.width; height: 72
            radius: Theme.radiusSmall; color: Theme.bgCard
            border { width: 1; color: Theme.borderLight }

            Row {
                anchors { left: parent.left; leftMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
                spacing: Theme.spacingSmall
                Icon { name: "person"; size: 28; color: model.role === "owner" ? Theme.primary : Theme.textHint; anchors.verticalCenter: parent.verticalCenter }
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2
                    Text { text: model.userId.substring(0, 8) + "..."; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
                    Text { text: model.role === "owner" ? "户主" : "成员"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
                }
            }

            // 操作按钮（仅户主可见）
            Row {
                anchors { right: parent.right; rightMargin: Theme.spacingSmall; verticalCenter: parent.verticalCenter }
                spacing: Theme.spacingSmall
                visible: isOwner && model.userId !== currentUserId

                Rectangle {
                    width: 70; height: 32; radius: Theme.radiusSmall
                    color: Theme.primary
                    Text { anchors.centerIn: parent; text: "移交"; font.pixelSize: Theme.fontSizeSmall; color: "white" }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: { transferDialog.targetUserId = model.userId; transferDialog.open() }
                    }
                }
                Rectangle {
                    width: 70; height: 32; radius: Theme.radiusSmall
                    color: Theme.danger
                    Text { anchors.centerIn: parent; text: "移除"; font.pixelSize: Theme.fontSizeSmall; color: "white" }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: { removeDialog.targetUserId = model.userId; removeDialog.open() }
                    }
                }
            }
        }

        Component.onCompleted: refresh()
    }

    property bool isOwner: {
        for (var i = 0; i < memberModel.count; i++) {
            var m = memberModel.get(i)
            if (m.userId === currentUserId) return m.role === "owner"
        }
        return false
    }

    function refresh() {
        memberModel.clear()
        var members = familyService.getMembers(familyId)
        for (var i = 0; i < members.length; i++) {
            memberModel.append(members[i])
        }
        // 获取邀请码
        var fams = familyService.getUserFamilies(currentUserId)
        if (fams.length > 0) inviteLabel.text = "邀请码: " + (fams[0].inviteCode || "无")
    }

    Text {
        id: inviteLabel
        anchors { bottom: parent.bottom; bottomMargin: 20; horizontalCenter: parent.horizontalCenter }
        text: ""
        font.pixelSize: Theme.fontSizeBody; color: Theme.primary
        visible: isOwner
    }

    // 移除成员确认
    Dialog {
        id: removeDialog
        property string targetUserId: ""
        title: "移除成员"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        Column { spacing: Theme.spacingSmall
            Text { text: "确认将该成员移出家庭？"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
        }
        onAccepted: {
            familyService.removeMember(familyId, targetUserId, currentUserId)
            refresh()
        }
    }

    // 户主移交确认
    Dialog {
        id: transferDialog
        property string targetUserId: ""
        title: "移交户主权"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        Column {
            spacing: Theme.spacingSmall
            Text { text: "确认将户主权移交给该成员？\n你将成为普通成员。"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
        }

        onAccepted: {
            var r = familyService.transferOwnership(familyId, currentUserId, targetUserId)
            if (r.success) refresh()
        }
    }
}
