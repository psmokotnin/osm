import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.2

Item {
    height: 50
    width: parent.width

    //property string propertiesQml: "qrc:/GeneratorProperties.qml"

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

    PropertiesOpener {
        propertiesQml: "qrc:/GeneratorProperties.qml"
    }
}
