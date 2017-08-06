import QtQuick 2.0

Item {
    anchors.fill: parent

    property string propertiesQml

    MouseArea {
        anchors.fill: parent
        onClicked: {
            applicationWindow.properiesbar.stack.clear()
            if (parent.propertiesQml)
                applicationWindow.properiesbar.stack.push(
                        parent.propertiesQml
                        )
            else
                console.error("qml not set for ", parent.parent)
        }
    }
}
