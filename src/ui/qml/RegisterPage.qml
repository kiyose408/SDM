import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0
import SmartDiet.Core 1.0

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
    property var registerData: ({})
    property string errorText: ""
    signal registrationComplete()
    signal backToLogin()
    signal nextStep()

    RegistrationValidator { id: validator }
    TdeeCalculator { id: tdeeCalculator }
    // authService 通过 main.cpp 的 setContextProperty 注入

    background: Rectangle { color: Theme.bgPage }

    // ---- 返回按钮 ----
    Item {
        anchors { top: parent.top; topMargin: Theme.spacingSmall; left: parent.left; leftMargin: Theme.spacingSmall }
        width: backRow.width; height: backRow.height

        Row {
            id: backRow
            spacing: 4
            Icon { name: "arrow_back"; size: 24; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "返回登录"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: root.backToLogin()
        }
    }

    Column {
        anchors {
            top: parent.top; topMargin: 40
            left: parent.left; leftMargin: Theme.spacingMedium
            right: parent.right; rightMargin: Theme.spacingMedium
            bottom: parent.bottom; bottomMargin: 70
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

        // ---- 密码 ----
        Rectangle {
            width: parent.width
            height: 52
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
                        text: "请设置登录密码"
                        font: parent.font
                        color: Theme.textHint
                        visible: !passwordInput.text && !passwordInput.activeFocus
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

        // ---- 错误反馈 ----
        Text {
            visible: root.errorText !== ""
            text: root.errorText
            width: parent.width
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.danger
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
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
                onClicked: {
                    var profile = {
                        nickname: nicknameInput.text.trim(),
                        password: passwordInput.text,
                        height: parseFloat(heightCard.input.text) || 0,
                        weight: parseFloat(weightCard.input.text) || 0,
                        age: parseInt(ageCard.input.text) || 0,
                        gender: root.gender,
                        dietGoal: root.dietGoal
                    };
                    var result = validator.validate(profile);
                    if (result.valid) {
                        // 保存 Step 1 数据到全局，进入 Step 2
                        registerData = {
                            nickname: nicknameInput.text.trim(),
                            password: passwordInput.text,
                            height: parseFloat(heightCard.input.text) || 0,
                            weight: parseFloat(weightCard.input.text) || 0,
                            age: parseInt(ageCard.input.text) || 0,
                            gender: root.gender,
                            dietGoal: root.dietGoal
                        };
                        root.nextStep();
                    } else {
                        console.log("Validation errors:", JSON.stringify(result.errors));
                    }
                }
            }
        }
    }
}
