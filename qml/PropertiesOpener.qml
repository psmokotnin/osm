import QtQuick 2.0

Item {
    anchors.fill: parent

    property string propertiesQml
    property var pushObject : false

    MouseArea {
        anchors.fill: parent
        onClicked: {
            applicationWindow.properiesbar.stack.clear()
            if (parent.propertiesQml) {
                var item = applicationWindow.properiesbar.stack.push(
                        parent.propertiesQml,
                        {
                            dataObject: pushObject
                        }
                        )
            }
            else
                console.error("qml not set for ", parent.parent)
        }
    }
}
