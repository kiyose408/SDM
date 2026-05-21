import QtQuick
import QtQuick.Controls
import SmartDiet.Style 1.0

Page {
    id: root
    title: "食材详情"
    property string ingredientId: ""

    background: Rectangle { color: Theme.bgPage }

    Component.onCompleted: {
        var data = ingredientService.getById(ingredientId)
        nameLabel.text     = data.name || "未命名"
        catLabel.text      = data.category || ""
        calLabel.text      = "热量: " + (data.caloriesPer100 || 0) + " kcal/100" + (data.unit || "g")
        proteinLabel.text  = "蛋白质: " + (data.proteinPer100 || 0) + " g"
        carbsLabel.text    = "碳水: " + (data.carbsPer100 || 0) + " g"
        fatLabel.text      = "脂肪: " + (data.fatPer100 || 0) + " g"
    }

    Item {
        anchors { top: parent.top; topMargin: Theme.spacingSmall; left: parent.left; leftMargin: Theme.spacingSmall }
        width: backRow.width; height: backRow.height
        Row {
            id: backRow; spacing: 4
            Icon { name: "arrow_back"; size: 24; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
            Text { text: "返回"; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary; anchors.verticalCenter: parent.verticalCenter }
        }
        MouseArea { anchors.fill: parent; onClicked: navStack.pop() }
    }

    Column {
        anchors { top: parent.top; topMargin: 50; left: parent.left; leftMargin: Theme.spacingMedium; right: parent.right; rightMargin: Theme.spacingMedium }
        spacing: Theme.spacingMedium

        Text { id: nameLabel; font.pixelSize: Theme.fontSizeLarge; color: Theme.textPrimary }
        Text { id: catLabel;  font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }

        Rectangle { width: parent.width; height: 1; color: Theme.borderLight }

        Text { id: calLabel;     font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
        Text { id: proteinLabel; font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
        Text { id: carbsLabel;   font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
        Text { id: fatLabel;     font.pixelSize: Theme.fontSizeBody; color: Theme.textPrimary }
    }
}
