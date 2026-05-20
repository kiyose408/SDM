import QtQuick
import QtQuick.Controls
import QtQuick.Window
import SmartDiet.Style 1.0

/**
 * @brief 注册独立窗口 — 无 TabBar，全屏沉浸体验
 *
 * 对应 003.详细设计文档 §2 界面一 + 界面二。
 * 用于首次启动或未登录状态，完成注册后切换到 main.qml。
 */
ApplicationWindow {
    id: root
    width: 420
    height: 680
    visible: true
    title: "SmartDietManager - 注册"
    color: Theme.bgPage

    FontLoader {
        source: "qrc:/fonts/MaterialIconsOutlined.otf"
    }

    RegisterPage {
        anchors.fill: parent
    }
}
