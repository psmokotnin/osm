import QtQuick 2.7

MouseArea {
    property string propertiesQml
    property var pushObject

    anchors.fill: parent
    onClicked: {
        applicationWindow.properiesbar.stack.clear()
        if (propertiesQml) {
            var item = applicationWindow.properiesbar.stack.push(
                    propertiesQml,
                    {
                        dataObject: pushObject
                    }
            )
        }
        else
            console.error("qml not set for ", parent.parent)
    }
}
