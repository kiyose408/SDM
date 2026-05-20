import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "我的"

    background: Rectangle { color: Theme.bgPage }

    onVisibleChanged: { if (visible) refreshFamilies() }

    function refreshFamilies() {
        familyListModel.clear()
        var fams = familyService.getUserFamilies(session.userId)
        for (var i = 0; i < fams.length; i++) familyListModel.append(fams[i])
    }

    ListModel { id: familyListModel }

    Column {
        anchors { top: parent.top; topMargin: Theme.spacingLarge; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: parent.bottom; bottomMargin: 80 }
        spacing: Theme.spacingMedium

        // 头像+昵称
        Icon { name: "person"; size: 48; color: Theme.primary; anchors.horizontalCenter: parent.horizontalCenter }
        Text { text: "个人中心"; font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary; anchors.horizontalCenter: parent.horizontalCenter }

        Rectangle { width: parent.width; height: 1; color: Theme.borderLight }

        // 我的家庭列表
        Text { text: "我的家庭 (" + familyListModel.count + ")"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }

        Repeater {
            model: familyListModel
            delegate: Rectangle {
                width: parent.width; height: 48; radius: Theme.radiusLarge
                color: Theme.bgCard; border { width: 1; color: Theme.borderLight }
                Row {
                    anchors { left: parent.left; leftMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
                    spacing: Theme.spacingSmall
                    Icon { name: "group"; size: 24; color: model.role === "owner" ? Theme.primary : Theme.textHint; anchors.verticalCenter: parent.verticalCenter }
                    Column { anchors.verticalCenter: parent.verticalCenter
                        Text { text: model.name; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
                        Text { text: model.role === "owner" ? "户主" : "成员"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
                    }
                }
                Text { text: ">"; font.pixelSize: Theme.fontSizeBody; color: Theme.textHint; anchors { right: parent.right; rightMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter } }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var comp = Qt.createComponent("qrc:/qml/MemberListPage.qml")
                        if (comp.status === Component.Ready) {
                            var page = comp.createObject(navStack, { familyId: model.familyId, currentUserId: session.userId })
                            if (page) navStack.push(page)
                        }
                    }
                }
            }
        }

        // 创建/加入新家庭
        Rectangle {
            width: parent.width; height: 44; radius: Theme.radiusLarge; color: Theme.primary
            Text { anchors.centerIn: parent; text: "+ 创建或加入新家庭"; font.pixelSize: Theme.fontSizeBody; color: "white" }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    var comp = Qt.createComponent("qrc:/qml/FamilySetupPage.qml")
                    if (comp.status === Component.Ready) {
                        var page = comp.createObject(navStack, { userId: session.userId, registrationMode: false })
                        page.onFamilyCreated.connect(function() { navStack.pop(); refreshFamilies() })
                        page.onFamilyJoined.connect(function() { navStack.pop(); refreshFamilies() })
                        navStack.push(page)
                    }
                }
            }
        }
    }

    // 退出登录
    Rectangle {
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
        height: 60; color: Theme.bgCard
        Rectangle {
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 1; color: Theme.borderLight
        }
        Row {
            anchors.centerIn: parent; spacing: Theme.spacingSmall
            Icon { name: "logout"; size: 22; color: Theme.danger; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "退出登录"; font.pixelSize: Theme.fontSizeBody; color: Theme.danger; anchors.verticalCenter: parent.verticalCenter }
        }
        MouseArea { anchors.fill: parent; onClicked: { session.clearSession(); Qt.callLater(Qt.quit) } }
    }
}
