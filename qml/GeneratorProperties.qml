import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.1

Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        //list of available output devices:
        ComboBox {
            id: deviceSelect
            model: generatorModel.devices
            implicitWidth: parent.width
            currentIndex: { model.indexOf(generatorModel.device) }
            onCurrentIndexChanged: generatorModel.device = model[currentIndex]
        }

        RowLayout {
            spacing: 10
            Layout.alignment: Qt.AlignTop

            //generator type
            ComboBox {
                currentIndex: generatorModel.type
                model: generatorModel.types
                onCurrentIndexChanged: generatorModel.type = currentIndex
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
