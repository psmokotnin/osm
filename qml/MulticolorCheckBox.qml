import QtQuick 2.9
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.2
import QtQuick.Controls.Material.impl 2.2

/**
 * Rewriting of QtQuick Controls 2 CheckBox
 * It puts colored rectangular under the original checkbox and check image
 * Also ripple effect is changed
 */
CheckBox {
    id: control

    property color checkedColor;

    //colored rectangular
    Rectangle {
        x: text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        z: 2

        width: 18
        height: 18
        opacity: control.checked ? 1 : 0
        color: control.checkedColor
    }

    //From source code. Only z value are changed
    Image {
        id: checkImage
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        z: 3
        width: 14
        height: 14
        source: "qrc:/qt-project.org/imports/QtQuick/Controls.2/Material/images/check.png"
        fillMode: Image.PreserveAspectFit

        scale: control.checkState === Qt.Checked ? 1 : 0
        Behavior on scale { NumberAnimation { duration: 100 } }
    }

    //From source code
    indicator: CheckIndicator {
        x: text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        control: control

        Ripple {
            id: ripple
             x: (parent.width - width) / 2
             y: (parent.height - height) / 2
             width: 28; height: 28

             z: -1
             anchor: control
             pressed: control.pressed
             active: control.down || control.visualFocus || control.hovered
         }
    }

    //Update Ripple color by checked state or color are changed
    function rippleColor() {
        var rippleCheckedColor = Qt.rgba(
                    control.checkedColor.r,
                    control.checkedColor.g,
                    control.checkedColor.b,
                    0.1
                    );
        ripple.color = control.checked ? rippleCheckedColor : control.Material.rippleColor
    }

    onCheckStateChanged: rippleColor()
    Component.onCompleted: rippleColor()
    onCheckedColorChanged: rippleColor()
}
