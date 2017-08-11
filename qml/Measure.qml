import QtQuick 2.0
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Item {
    id: measure

    property var m;
    height: 50
    width: parent.width

    RowLayout {
        width: parent.width

        CheckBox {
            anchors.verticalCenter: parent.verticalCenter

            onCheckStateChanged: {
                m.active = checked
            }
            Component.onCompleted: {
                checked = m.active
            }

        }

        ColumnLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                text:  m.name//qsTr("Measure")

                PropertiesOpener {
                   propertiesQml: "qrc:/MeasureProperties.qml"
                   pushObject: measure.m
                }
            }
            ProgressBar {
                value: m.level
                implicitWidth: parent.width
            }
        }
    }
}
