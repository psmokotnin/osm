import QtQuick 2.7

MouseArea {
    property string propertiesQml
    property var pushObject

    anchors.fill: parent
    onClicked: {
        var aw = applicationWindow;
        aw.properiesbar.stack.clear();

        if (propertiesQml) {
            var item = aw.properiesbar.stack.push(
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
