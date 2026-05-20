import QtQuick
import QtQuick.Controls
import QtQuick.Window
import SmartDiet.Style 1.0

ApplicationWindow {
    id: root
    width: 420
    height: 680
    visible: true
    title: "SmartDietManager - 注册"
    color: Theme.bgPage

    FontLoader { source: "qrc:/fonts/MaterialIconsOutlined.otf" }

    property string registeredUserId: ""

    StackView {
        id: regStack
        anchors.fill: parent
        initialItem: step1

        Component {
            id: step1
            RegisterPage {
                onNextStep: {
                    // Step 1 校验通过 → 注册用户
                    var d = registerData
                    var result = authService.registerUser(
                        d.nickname, d.password,
                        { nickname: d.nickname, password: d.password, gender: d.gender,
                          height: d.height, weight: d.weight, age: d.age, dietGoal: d.dietGoal })
                    if (result.success) {
                        root.registeredUserId = result.userId
                        regStack.push(step2)
                    } else {
                        console.log("Register failed in Step 1:", JSON.stringify(result))
                    }
                }
            }
        }

        Component {
            id: step2
            FamilySetupPage {
                userId: root.registeredUserId
                onBackToStep1: regStack.pop()
                onFamilyCreated: root.close()
                onFamilyJoined:  root.close()
            }
        }
    }
}
