import QtQuick 2.0

import QtQuick 2.0
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

Popup {
    id: popup
    property string text : "";
    property color color: "transparent"
    width: 300
    height: 100
    x: (parent ? (parent.width  - width) / 2 : 100)
    y: (parent ? (parent.height - height) / 2 : 100)

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    background: null

    Rectangle {
        anchors.fill: parent
        border.width: 4
        radius: 4
        border.color: popup.color;

        Text {
            id: update;
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
            textFormat: Text.RichText
            text: popup.text
        }
    }

    function showError(message) {
        color = Material.color(Material.Red);
        text = message;
        open();
    }
}
