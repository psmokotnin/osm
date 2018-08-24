import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Item {
    id: measurement

    property var dataModel;
    property bool chartable : true;
    height: 50
    width: (parent ? parent.width : 0)

    RowLayout {
        width: parent.width

        MulticolorCheckBox {
            id: checkbox
            Layout.alignment: Qt.AlignVCenter

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
                   propertiesQml: "qrc:/MeasurementProperties.qml"
                   pushObject: measurement.dataModel
                }
            }

            Meter {
                dBV: dataModel.level
                width: parent.width
            }

            Meter {
                dBV: dataModel.referenceLevel
                width: parent.width
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
