import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "我的"

    Rectangle {
        anchors.fill: parent
        color: Theme.bgPage

        Column {
            anchors.centerIn: parent
            spacing: Theme.spacingMedium

            Icon {
                name: "person"
                size: 48
                color: Theme.primary
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: "个人中心"
                font.pixelSize: Theme.fontSizeTitle
                color: Theme.textPrimary
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }

    // 底部登出按钮
    Rectangle {
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
        height: 60
        color: Theme.bgCard

        Rectangle {
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 1; color: Theme.borderLight
        }

        Row {
            anchors.centerIn: parent
            spacing: Theme.spacingSmall
            Icon { name: "logout"; size: 22; color: Theme.danger; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "退出登录"; font.pixelSize: Theme.fontSizeBody; color: Theme.danger; anchors.verticalCenter: parent.verticalCenter }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                session.clearSession()
                Qt.callLater(Qt.quit)  // 延迟退出，让 QML 先完成当前帧
            }
        }
    }
}
