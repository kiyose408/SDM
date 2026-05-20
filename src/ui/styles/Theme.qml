pragma Singleton
import QtQuick

/**
 * @brief 全局主题单例 — 色彩、字体、间距、动效常量
 *
 * 数据来源：003.详细设计文档 §1 色彩体系 + 004.数据库设计文档枚举
 * 使用方式：import SmartDiet.Style 1.0 → Theme.primaryColor
 *
 * 严格禁止：任何 QML 组件中硬编码色值或字号，必须通过此单例引用
 */
QtObject {
    // ================================================================
    // 色彩（对齐 src/utils/constants.h ThemeColor 命名空间）
    // ================================================================
    readonly property color primary:      "#64C288"   // 暖柔青柠绿（主色）
    readonly property color secondary:    "#FF9A5A"   // 活力橙
    readonly property color danger:       "#E86B6B"   // 豆沙红（预警/错误）
    readonly property color accent:       "#F9D07B"   // 暖杏黄
    readonly property color info:         "#72B5E4"   // 浅湖蓝
    readonly property color textPrimary:  "#333333"   // 主文字深灰
    readonly property color textHint:     "#999999"   // 辅助文字浅灰
    readonly property color bgPage:       "#F5F5F5"   // 页面背景
    readonly property color bgCard:       "#FFFFFF"   // 卡片背景纯白
    readonly property color borderLight:  "#E5E7EB"   // 浅灰边框（输入框用）

    // ================================================================
    // 字体大小
    // ================================================================
    readonly property int fontSizeSmall:  12
    readonly property int fontSizeBody:   14
    readonly property int fontSizeTitle:  18
    readonly property int fontSizeLarge:  24

    // ================================================================
    // 间距
    // ================================================================
    readonly property int spacingSmall:   8
    readonly property int spacingMedium:  16
    readonly property int spacingLarge:   24

    // ================================================================
    // 圆角
    // ================================================================
    readonly property int radiusSmall:    8    // 胶囊标签、小按钮
    readonly property int radiusMedium:   12   // 半屏抽屉顶部
    readonly property int radiusLarge:    16   // 大按钮、输入框

    // ================================================================
    // 动效时长（毫秒）
    // ================================================================
    readonly property int durationFast:     150   // 按钮点击反馈
    readonly property int durationNormal:   300   // 半屏抽屉、页面切换
    readonly property int durationSlow:     900   // 环形色块生长
}
