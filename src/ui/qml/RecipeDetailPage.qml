import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root; title: ""; property string recipeId: ""
    background: Rectangle { color: Theme.bgPage }

    Component.onCompleted: {
        var data = recipeService.getById(recipeId)
        nameLabel.text = data.name||""; descLabel.text = data.description||""
        timeLabel.text = "烹饪时间: "+(data.cookingTime||0)+" min | "+(data.servings||2)+" 人份"
        calLabel.text = "热量: "+(data.totalCalories||0).toFixed(0)+" kcal"
        proLabel.text = "蛋白质: "+(data.totalProtein||0).toFixed(1)+" g"
        carbLabel.text = "碳水: "+(data.totalCarbs||0).toFixed(1)+" g"
        fatLabel.text = "脂肪: "+(data.totalFat||0).toFixed(1)+" g"

        var ings = recipeService.getIngredients(recipeId)
        for (var i=0;i<ings.length;i++) ingModel.append(ings[i])
    }

    Item {
        anchors { top: parent.top; topMargin: Theme.spacingSmall; left: parent.left; leftMargin: Theme.spacingSmall }
        width: backRow.width; height: backRow.height
        Row { id: backRow; spacing: 4
            Icon { name: "arrow_back"; size: 24; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "返回"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
        }
        MouseArea { anchors.fill: parent; onClicked: navStack.pop() }
    }

    ListModel { id: ingModel }

    ScrollView {
        anchors { top: parent.top; topMargin: 40; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium; bottom: parent.bottom }
        Column { spacing: Theme.spacingMedium
            Text { id: nameLabel; font.pixelSize: Theme.fontSizeLarge; color: Theme.textPrimary }
            Text { id: descLabel; font.pixelSize: Theme.fontSizeBody; color: Theme.textHint }
            Text { id: timeLabel; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
            Rectangle { width: parent.width; height: 1; color: Theme.borderLight }
            Text { text: "营养成分"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Text { id: calLabel; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Text { id: proLabel; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Text { id: carbLabel; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Text { id: fatLabel; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Rectangle { width: parent.width; height: 1; color: Theme.borderLight }
            Text { text: "食材"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            Repeater {
                model: ingModel
                delegate: Text { text: model.name+" · "+model.amount+model.unit; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
            }
        }
    }
}
