import QtQuick
import SmartDiet.Style 1.0

Rectangle {
    id: root
    property string text: ""
    property int value: 1
    property bool selected: false

    signal clicked(int value)

    width: (parent.width - Theme.spacingSmall) / 2
    height: 48
    radius: Theme.radiusSmall
    color: selected ? Qt.rgba(100/255, 194/255, 136/255, 0.10) : Theme.bgCard
    border { width: 1; color: selected ? Theme.primary : Theme.borderLight }

    Icon {
        name: "person"
        size: 24
        color: Theme.primary
        anchors { left: parent.left; leftMargin: Theme.spacingMedium; verticalCenter: parent.verticalCenter }
        opacity: selected ? 1.0 : 0.3
    }

    Text {
        text: root.text
        font.pixelSize: Theme.fontSizeBody
        color: Theme.textPrimary
        anchors { left: parent.left; leftMargin: 52; verticalCenter: parent.verticalCenter }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked(root.value)
    }
}
