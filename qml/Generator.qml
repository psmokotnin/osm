import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

import QtQuick.Templates 2.2 as T
import QtQuick.Controls.Material 2.2
//import QtQuick.Controls.Material.impl 2.2

Item {
    id: generator

    GroupBox {
        title: qsTr("Generator")

        Row {
            spacing: 5

            Switch {
                checked: generatorModel.enabled
                onCheckedChanged: generatorModel.enabled = checked
            }

            Button {
                text: qsTr("Settings")
                checked: generatorModel.enabled
                highlighted: checked
                onClicked: {
                    //first open for calc width and height of popup
                    popup.open();
                    popup.x = (generator.parent.width - popup.width) / 2
                    popup.y = (generator.parent.height - popup.height) / 2

                }
            }
        }
    }

    /**
     * Generator settings dialog
     */
    Dialog {
        id: popup
        width: 400
        title: qsTr("Generator settings")
        modal: true
        standardButtons: Dialog.Apply | Dialog.Cancel

        ColumnLayout {
            spacing: 10
            implicitWidth: popup.width - (popup.padding * 2)
            anchors.left: parent.Left
            anchors.right: parent.Right

            //list of available output devices:
            ComboBox {
                id: deviceSelect
                model: generatorModel.devices
                implicitWidth: parent.width
            }

            //generator type
            ComboBox {
                width: 200
                currentIndex: generatorModel.type
                model: generatorModel.types
                onCurrentIndexChanged: generatorModel.type = currentIndex
                implicitWidth: parent.width
            }

            //Sin frequency
            SpinBox {
                id: frequencySpinBox
                value: generatorModel.frequency
                from: 20
                to: 20000
                editable: true
                onValueChanged: generatorModel.frequency = value
            }
        }
    }
}
