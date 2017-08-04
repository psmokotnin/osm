import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.2

Item {
    id: generator
    height: 50
    width: parent.width

    MouseArea {
        anchors.fill: parent
        onClicked: {
            applicationWindow.properiesbar.stack.clear()
            applicationWindow.properiesbar.stack.push(
                        "qrc:/GeneratorProperties.qml"
                        )
        }
    }

    Row {
        height: 50

        Switch {
            anchors.verticalCenter: parent.verticalCenter

            checked: generatorModel.enabled
            onCheckedChanged: generatorModel.enabled = checked
        }

        Label {
            anchors.verticalCenter: parent.verticalCenter
            text:  qsTr("Generator")
        }
    }
}
