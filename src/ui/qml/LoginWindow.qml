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
            var comp = Qt.createComponent("qrc:/qml/RegisterWindow.qml")
            if (comp.status === Component.Ready) {
                var w = comp.createObject(root)
                if (w) {
                    w.onClosing.connect(function() { root.show() })
                    root.hide()
                    w.show()
                }
            } else {
                console.log("Failed to load RegisterWindow:", comp.errorString())
            }
        }
        onLoggedIn: {
            // C++ 侧 session::loggedIn 信号会关闭本窗口并加载 main.qml
        }
    }

}
