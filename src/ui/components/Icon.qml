import QtQuick
import SmartDiet.Style 1.0

/**
 * @brief Material Icons 图标组件
 *
 * 基于 Google Material Icons Outlined 字体（OpenType ligatures）。
 * 用法：<Icon name="home" size="24" color="Theme.primary" />
 *
 * 图标名称参考：https://fonts.google.com/icons?icon.style=Outlined
 * 常用图标：home / kitchen / menu_book / person / settings / search /
 *           add / edit / delete / close / arrow_back / arrow_forward /
 *           check / shopping_cart / calendar_today / favorite / notifications
 */
Text {
    id: root

    /// 图标名称（Material Icons 官方名称，全小写下划线分隔）
    property string name: ""

    /// 图标大小（像素），默认继承 Theme.fontSizeTitle
    property real size: Theme.fontSizeTitle

    /// 图标颜色，默认继承 Theme.textPrimary
    property color color: Theme.textPrimary

    // 渲染
    text: name
    font.family: "Material Icons Outlined"
    font.pixelSize: size
    color: root.color

    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    renderType: Text.NativeRendering  // 字体图标用 NativeRendering 更清晰
}
