import QtQuick
import SmartDiet.Style 1.0

/**
 * @brief 注册页 — 身体数据卡片（身高/体重/年龄）
 */
Rectangle {
    id: root
    property alias input: textInput
    property string label: ""

    width: (parent.width - Theme.spacingSmall * 2) / 3
    height: 72
    radius: Theme.radiusSmall
    color: Theme.bgCard
    border { width: 1; color: Theme.borderLight }

    Column {
        anchors.centerIn: parent
        spacing: 4

        Text {
            text: root.label
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.textHint
            anchors.horizontalCenter: parent.horizontalCenter
        }
        TextInput {
            id: textInput
            width: root.width - 16
            font.pixelSize: Theme.fontSizeTitle
            color: Theme.textPrimary
            horizontalAlignment: TextInput.AlignHCenter
            validator: DoubleValidator {}

            Text {
                text: "0"
                font: parent.font
                color: Theme.textHint
                anchors.centerIn: parent
                visible: !textInput.text && !textInput.activeFocus
            }
        }
    }
}
