import QtQuick 2.7
import QtQuick.Dialogs 1.2

Item {
    id: picker
    property color color

    Rectangle {
        id: preview
        width: picker.width
        height: picker.height
        color: picker.color
    }

    MouseArea {
        anchors.fill: preview
        cursorShape: "PointingHandCursor"
        onClicked: colorDialog.open()
    }

    ColorDialog {
        id: colorDialog
        title: qsTr("Please choose a color")
        color: picker.color

        onAccepted: {
            picker.color = color
        }
    }
}
