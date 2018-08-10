import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Item {
    id: measure

    property var dataModel;
    property bool chartable : true;
    height: 50
    width: parent.width

    RowLayout {
        width: parent.width

        MulticolorCheckBox {
            id: checkbox
            anchors.verticalCenter: parent.verticalCenter

            checkedColor: dataModel.color

            onCheckStateChanged: {
                dataModel.active = checked
            }
            Component.onCompleted: {
                checked = dataModel.active
            }
        }

        ColumnLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                text:  dataModel.name

                PropertiesOpener {
                   propertiesQml: "qrc:/MeasureProperties.qml"
                   pushObject: measure.dataModel
                }
            }

            ProgressBar {
                value: dataModel.level
                implicitWidth: parent.width
            }

            ProgressBar {
                value: dataModel.referenceLevel
                implicitWidth: parent.width
            }
        }

        Connections {
            target: dataModel
            onColorChanged: checkbox.checkedColor = dataModel.color
        }

        Component.onCompleted: {
            dataModel.color = applicationWindow.dataSourceList.nextColor();
        }
    }
}
