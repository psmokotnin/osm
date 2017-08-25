import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Item {
    id: store

    property var dataModel;
    property bool chartable : true;
    height: 50
    //width: parent.width

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
                   propertiesQml: "qrc:/StoredProperties.qml"
                   pushObject: store.dataModel
                }
            }

        }

        Connections {
            target: dataModel
            onColorChanged: checkbox.checkedColor = dataModel.color
        }
    }
}
