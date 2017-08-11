import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Item {

    property var dataObject

    RowLayout {
        spacing: 0

        SpinBox {
            //id: frequencySpinBox
            value: dataObject.pointsPerOctave
            from: 0
            to: 24
            //editable: true
            onValueChanged: dataObject.pointsPerOctave = value
        }

        TextField {
            placeholderText: qsTr("title")
            text: dataObject.name
            onTextEdited: dataObject.name = text

        }

        ColorPicker {
            id: colorPicker

            Layout.preferredWidth: 25
            Layout.preferredHeight: 25

            onColorChanged: {
                dataObject.color = color
            }
        }

        Component.onCompleted: {
            colorPicker.color = dataObject.color
        }
    }
}
