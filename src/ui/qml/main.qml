import QtQuick
import QtQuick.Controls
import QtQuick.Window
import SmartDiet.Style 1.0

ApplicationWindow {
    id: root
    width: 800
    height: 600
    visible: true
    title: "SmartDietManager"
    color: Theme.bgPage

    // ================================================================
    // 页面栈（主内容区）
    // ================================================================
    StackView {
        id: navStack
        anchors.fill: parent
        anchors.bottomMargin: tabBar.height  // 为底部导航留空间
        initialItem: homePage
    }

    // ================================================================
    // 底部导航栏
    // ================================================================
    footer: TabBar {
        id: tabBar
        currentIndex: 0

        background: Rectangle {
            color: Theme.bgCard
            // 顶部分割线
            Rectangle {
                anchors { left: parent.left; right: parent.right; top: parent.top }
                height: 1
                color: Theme.borderLight
            }
        }

        TabButton {
            text: "🏠\n首页"
            font.pixelSize: Theme.fontSizeSmall
        }
        TabButton {
            text: "🧊\n冰箱"
            font.pixelSize: Theme.fontSizeSmall
        }
        TabButton {
            text: "📖\n菜谱"
            font.pixelSize: Theme.fontSizeSmall
        }
        TabButton {
            text: "👤\n我的"
            font.pixelSize: Theme.fontSizeSmall
        }

        onCurrentIndexChanged: {
            // 无动画切换主页面（Tab 切换不应有推入动画）
            navStack.replace(null, pages[currentIndex], StackView.Immediate);
        }
    }

    // ================================================================
    // 页面实例（预创建，Tab 切换时不丢失状态）
    // ================================================================
    readonly property var pages: [homePage, fridgePage, recipesPage, profilePage]

    HomePage    { id: homePage    }
    FridgePage  { id: fridgePage  }
    RecipesPage { id: recipesPage }
    ProfilePage { id: profilePage }
}
