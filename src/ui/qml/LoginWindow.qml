import QtQuick
import QtQuick.Controls
import QtQuick.Window
import SmartDiet.Style 1.0

ApplicationWindow {
    id: root
    width: 420
    height: 480
    visible: true
    title: "SmartDietManager - 登录"
    color: Theme.bgPage

    FontLoader { source: "qrc:/fonts/MaterialIconsOutlined.otf" }

    LoginPage {
        anchors.fill: parent
        onNavigateToRegister: {
            registerWindow.show()
            root.hide()
        }
        onLoggedIn: {
            // C++ 侧 session::loggedIn 信号会关闭本窗口并加载 main.qml
        }
    }

    // 注册子窗口（独立 Window，初始隐藏）
    Window {
        id: registerWindow
        width: 420
        height: 680
        visible: false
        title: "SmartDietManager - 注册"
        color: Theme.bgPage
        flags: Qt.Dialog

        FontLoader { source: "qrc:/fonts/MaterialIconsOutlined.otf" }

        RegisterPage {
            anchors.fill: parent
            onBackToLogin: {
                registerWindow.hide()
                root.show()
            }
            onRegistrationComplete: {
                registerWindow.hide()
                root.show()
            }
        }
    }
}
