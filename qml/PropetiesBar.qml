import QtQuick 2.7
import QtQuick.Controls 2.2

Item {
    property alias stack: propertiesStack

    StackView {
        id: propertiesStack
        anchors.fill: parent
        anchors.margins: 5
        initialItem: topView
    }

    Component {
        id: topView

        Label {
            anchors.fill: parent

            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter

            text: qsTr("Select an item in the right bar to change its properties")
        }
    }
}
