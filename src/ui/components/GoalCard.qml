import QtQuick
import SmartDiet.Style 1.0

Rectangle {
    id: root
    property string title: ""
    property string subtitle: ""
    property int value: 0
    property bool selected: false

    signal clicked(int value)

    width: parent.width
    height: 56
    radius: Theme.radiusSmall
    color: selected ? Qt.rgba(100/255, 194/255, 136/255, 0.10) : Theme.bgCard
    border { width: 1; color: selected ? Theme.primary : Theme.borderLight }

    Row {
        anchors { left: parent.left; leftMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
        spacing: Theme.spacingSmall
        Column {
            spacing: 2
            Text { text: root.title;    font.pixelSize: Theme.fontSizeBody;  color: Theme.textPrimary }
            Text { text: root.subtitle; font.pixelSize: Theme.fontSizeSmall; color: Theme.textHint }
        }
    }

    Icon {
        name: "check"
        size: 20
        color: Theme.primary
        anchors { right: parent.right; rightMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
        opacity: selected ? 1.0 : 0.0
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked(root.value)
    }
}
