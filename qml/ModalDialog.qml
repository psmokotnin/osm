import QtQuick 2.12
import QtQuick.Controls 2.15

Dialog {
    id: dialog
    title: ""
    modal: true
    focus: true
    x: (parent ? (parent.width  - width) / 2 : 100)
    y: (parent ? (parent.height - height) / 2 : 100)
    standardButtons: Dialog.Ok | Dialog.Cancel
    onAccepted: {}
    onRejected: {}

    Item {
        focus: true
        anchors.fill: parent
        visible: dialog.visible
        Keys.onReturnPressed: {
            dialog.accept();
        }
    }
}
