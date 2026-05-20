import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

/**
 * @brief 注册 Step 2 — 家庭关系绑定
 *
 * 对应 003.详细设计文档 §2 界面二。
 */
Page {
    id: root
    title: ""

    signal familyCreated()
    signal familyJoined()
    signal backToStep1()
    property string userId: ""
    property bool registrationMode: true  // true=注册流程, false=独立使用

    property alias createResult: createResult

    background: Rectangle { color: Theme.bgPage }

    // ---- 返回按钮 ----
    Item {
        anchors { top: parent.top; topMargin: Theme.spacingSmall; left: parent.left; leftMargin: Theme.spacingSmall }
        width: backRow.width; height: backRow.height
        Row {
            id: backRow; spacing: 4
            Icon { name: "arrow_back"; size: 24; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
            Text {
                text: root.registrationMode ? "上一步" : "返回"
                font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter
            }
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (root.registrationMode) root.backToStep1()
                else navStack.pop()
            }
        }
    }

    Column {
        anchors { top: parent.top; topMargin: 60; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: parent.bottom; bottomMargin: Theme.spacingMedium }
        spacing: Theme.spacingMedium

        // ---- 进度（仅注册模式） ----
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: root.registrationMode
            Text { text: "2/2"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint; anchors.horizontalCenter: parent.horizontalCenter }
            Text { text: "创建或加入你的家庭小组"; font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary; anchors.horizontalCenter: parent.horizontalCenter }
        }

        Item { width: 1; height: Theme.spacingLarge }

        // ---- 创建新家庭（主流程） ----
        Rectangle {
            width: parent.width; height: 52
            radius: Theme.radiusLarge; color: Theme.primary
            Text { anchors.centerIn: parent; text: "创建新家庭"; font.pixelSize: Theme.fontSizeBody; color: "white" }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var r = familyService.createFamily(root.userId, "")
                    createResult.text = r.success
                        ? "家庭创建成功！邀请码: " + r.inviteCode
                        : r.error || "创建失败"
                    if (r.success) root.familyCreated()
                }
            }
        }

        // ---- 加入已有家庭（次流程） ----
        Rectangle {
            width: parent.width; height: 52
            radius: Theme.radiusLarge
            color: "transparent"
            border { width: 1; color: Theme.primary }
            Text { anchors.centerIn: parent; text: "加入已有家庭"; font.pixelSize: Theme.fontSizeBody; color: Theme.primary }
            MouseArea {
                anchors.fill: parent
                onClicked: joinDrawer.open()
            }
        }

        // ---- 结果提示 ----
        Text {
            id: createResult
            width: parent.width
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.primary
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        // ---- 辅助说明 ----
        Text {
            text: "您可以随时在个人中心邀请家人加入"
            font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    // ================================================================
    // 加入家庭 — 底部抽屉
    // ================================================================
    Drawer {
        id: joinDrawer
        width: root.width
        height: 260
        edge: Qt.BottomEdge
        interactive: true
        dragMargin: 0

        background: Rectangle {
            color: Theme.bgCard
            radius: Theme.radiusMedium
        }

        Column {
            anchors { fill: parent; margins: Theme.spacingMedium }
            spacing: Theme.spacingMedium

            Text {
                text: "输入邀请码"
                font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Rectangle {
                width: parent.width; height: 52
                radius: Theme.radiusLarge
                color: Theme.bgPage
                border { width: 1; color: Theme.borderLight }

                TextInput {
                    id: codeInput
                    anchors { fill: parent; leftMargin: Theme.spacingMedium; rightMargin: Theme.spacingMedium }
                    font.pixelSize: 28; color: Theme.textPrimary
                    horizontalAlignment: TextInput.AlignHCenter
                    maximumLength: 6
                    inputMethodHints: Qt.ImhUppercaseOnly
                    Text {
                        text: "6 位邀请码"; font.pixelSize: 20; color: Theme.textHint
                        anchors.centerIn: parent
                        visible: !codeInput.text && !codeInput.activeFocus
                    }
                }
            }

            Text {
                id: joinError
                width: parent.width
                font.pixelSize: Theme.fontSizeSmall; color: Theme.danger
                horizontalAlignment: Text.AlignHCenter
                visible: text !== ""
            }

            Rectangle {
                width: parent.width; height: 44
                radius: Theme.radiusLarge; color: codeInput.text.length === 6 ? Theme.primary : Theme.textHint
                Text { anchors.centerIn: parent; text: "确认加入"; font.pixelSize: Theme.fontSizeBody; color: "white" }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        joinError.text = ""
                        if (codeInput.text.length < 6) {
                            joinError.text = "请输入 6 位邀请码"; return
                        }
                        var r = familyService.joinFamily(root.userId, codeInput.text.toUpperCase())
                        if (r.success) {
                            joinDrawer.close()
                            root.familyJoined()
                        } else {
                            joinError.text = r.error || "加入失败"
                        }
                    }
                }
            }
        }
    }
}
