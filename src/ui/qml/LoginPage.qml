import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0
import SmartDiet.Core 1.0

/**
 * @brief 登录页面
 *
 * 对应 003.详细设计文档 登录模块。
 * 输入 login_id + 密码，点击登录调用 AuthService::login()。
 */
Page {
    id: root
    title: ""
    signal navigateToRegister()
    signal loggedIn()

    background: Rectangle { color: Theme.bgPage }

    // ================================================================
    // 标题区
    // ================================================================
    Column {
        id: header
        anchors { top: parent.top; topMargin: 60; horizontalCenter: parent.horizontalCenter }
        spacing: Theme.spacingMedium

        Icon {
            name: "person"
            size: 56
            color: Theme.primary
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: "欢迎回来"
            font.pixelSize: Theme.fontSizeLarge
            color: Theme.textPrimary
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    // ================================================================
    // 表单区
    // ================================================================
    Column {
        anchors {
            top: header.bottom; topMargin: 40
            left: parent.left; leftMargin: Theme.spacingMedium
            right: parent.right; rightMargin: Theme.spacingMedium
        }
        spacing: Theme.spacingMedium

        // ---- login_id ----
        Rectangle {
            width: parent.width; height: 52
            radius: Theme.radiusLarge
            color: Theme.bgCard
            border { width: 1; color: Theme.borderLight }

            Row {
                anchors { left: parent.left; leftMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
                spacing: Theme.spacingSmall
                Icon { name: "person"; size: 22; color: Theme.textHint; anchors.verticalCenter: parent.verticalCenter }
                TextInput {
                    id: loginIdInput
                    width: parent.parent.width - 80
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    Text {
                        text: "手机号 / 邮箱 / 用户名"
                        font: parent.font
                        color: Theme.textHint
                        visible: !loginIdInput.text && !loginIdInput.activeFocus
                    }
                }
            }
        }

        // ---- 密码 ----
        Rectangle {
            width: parent.width; height: 52
            radius: Theme.radiusLarge
            color: Theme.bgCard
            border { width: 1; color: Theme.borderLight }

            Row {
                anchors { left: parent.left; leftMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
                spacing: Theme.spacingSmall
                Icon { name: "lock"; size: 22; color: Theme.textHint; anchors.verticalCenter: parent.verticalCenter }
                TextInput {
                    id: passwordInput
                    width: parent.parent.width - 80
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    echoMode: TextInput.Password
                    Text {
                        text: "请输入密码"
                        font: parent.font
                        color: Theme.textHint
                        visible: !passwordInput.text && !passwordInput.activeFocus
                    }
                }
            }
        }

        // ---- 错误提示 ----
        Text {
            id: errorText
            width: parent.width
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.danger
            visible: text !== ""
            horizontalAlignment: Text.AlignHCenter
        }

        // ---- 登录按钮 ----
        Rectangle {
            width: parent.width; height: 44
            radius: Theme.radiusLarge
            color: Theme.primary

            Text {
                anchors.centerIn: parent
                text: "登 录"
                font.pixelSize: Theme.fontSizeBody
                color: "white"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    errorText.text = "";
                    if (!loginIdInput.text.trim()) {
                        errorText.text = "请输入账号";
                        return;
                    }
                    if (!passwordInput.text) {
                        errorText.text = "请输入密码";
                        return;
                    }
                    var result = authService.login(loginIdInput.text.trim(), passwordInput.text);
                    if (result.success) {
                        session.setSession(result.userId, result.nickname);
                        root.loggedIn();
                    } else {
                        errorText.text = result.error || "登录失败";
                    }
                }
            }
        }

        // ---- 注册入口 ----
        Text {
            text: "还没有账号？去注册"
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.primary
            anchors.horizontalCenter: parent.horizontalCenter
            MouseArea {
                anchors.fill: parent
                onClicked: root.navigateToRegister()
            }
        }
    }
}
