import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

/**
 * @brief 注册 Step 1 — 个人档案建立
 *
 * 对应 003.详细设计文档 §2 界面一。
 */
Page {
    id: root
    title: ""

    property int gender: 0       // 0=未选, 1=男, 2=女
    property int dietGoal: -1    // 0=减脂, 1=增肌, 2=维持

    background: Rectangle { color: Theme.bgPage }

    Column {
        anchors {
            top: parent.top; topMargin: Theme.spacingLarge
            left: parent.left; leftMargin: Theme.spacingMedium
            right: parent.right; rightMargin: Theme.spacingMedium
            bottom: parent.bottom; bottomMargin: 70  // 留给底部按钮
        }
        spacing: Theme.spacingMedium

        // ---- 进度 ----
        Column {
            anchors.horizontalCenter: parent.horizontalCenter
            Text { text: "1/2"; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint; anchors.horizontalCenter: parent.horizontalCenter }
            Text { text: "先让我们了解你"; font.pixelSize: Theme.fontSizeTitle; color: Theme.textPrimary; anchors.horizontalCenter: parent.horizontalCenter }
        }

        // ---- 昵称 ----
        Rectangle {
            width: parent.width
            height: 52
            radius: Theme.radiusLarge
            color: Theme.bgCard
            border { width: 1; color: Theme.borderLight }

            Row {
                anchors { left: parent.left; leftMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
                spacing: Theme.spacingSmall
                Icon { name: "person"; size: 22; color: Theme.textHint; anchors.verticalCenter: parent.verticalCenter }
                TextInput {
                    id: nicknameInput
                    width: parent.parent.width - 80
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: Theme.fontSizeBody
                    color: Theme.textPrimary
                    Text {
                        text: "请输入您的昵称"
                        font: parent.font
                        color: Theme.textHint
                        visible: !nicknameInput.text && !nicknameInput.activeFocus
                    }
                }
            }
        }

        // ---- 身高 / 体重 / 年龄 ----
        Row {
            width: parent.width
            spacing: Theme.spacingSmall
            BodyDataCard { id: heightCard; label: "身高/cm" }
            BodyDataCard { id: weightCard; label: "体重/kg" }
            BodyDataCard { id: ageCard;    label: "年龄/岁" }
        }

        // ---- 性别 ----
        Text { text: "性别"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
        Row {
            width: parent.width
            spacing: Theme.spacingSmall
            GenderButton {
                text: "男生"; value: 1; selected: root.gender === 1
                onClicked: function(v) { root.gender = v }
            }
            GenderButton {
                text: "女生"; value: 2; selected: root.gender === 2
                onClicked: function(v) { root.gender = v }
            }
        }

        // ---- 饮食目标 ----
        Text { text: "饮食目标"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
        GoalCard {
            title: "健康减脂"; subtitle: "控制热量摄入，科学减重"; value: 0
            selected: root.dietGoal === 0; onClicked: function(v) { root.dietGoal = v }
        }
        GoalCard {
            title: "增肌塑形"; subtitle: "增加蛋白质摄入，配合锻炼"; value: 1
            selected: root.dietGoal === 1; onClicked: function(v) { root.dietGoal = v }
        }
        GoalCard {
            title: "日常维持"; subtitle: "保持当前体重，均衡饮食"; value: 2
            selected: root.dietGoal === 2; onClicked: function(v) { root.dietGoal = v }
        }
    }

    // ---- 底部按钮 ----
    Rectangle {
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
        height: 64
        color: Theme.bgCard

        Rectangle {
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 1
            color: Theme.borderLight
        }

        Rectangle {
            anchors.centerIn: parent
            width: parent.width - Theme.spacingMedium * 2
            height: 44
            radius: Theme.radiusLarge
            color: Theme.primary

            Text { anchors.centerIn: parent; text: "下一步"; font.pixelSize: Theme.fontSizeBody; color: "white" }

            MouseArea {
                anchors.fill: parent
                onClicked: console.log("Register Step 1 → next")
            }
        }
    }
}
