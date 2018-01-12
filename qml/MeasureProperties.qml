import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Item {

    property var dataObject

    ColumnLayout {
        spacing: 0

    RowLayout {
        spacing: 0

        SpinBox {
            implicitWidth: 125
            value: dataObject.pointsPerOctave
            from: 0
            to: 48
            editable: true
            onValueChanged: dataObject.pointsPerOctave = value
        }

        SpinBox {
            implicitWidth: 125
            value: dataObject.average
            from: 1
            to: 100
            editable: true
            onValueChanged: dataObject.average = value
        }

        SpinBox {
            implicitWidth: 225
            value: dataObject.delay
            from: 0
            to: 48000
            editable: true
            onValueChanged: dataObject.delay = value

            textFromValue: function(value, locale) {
                return Number(value / 48).toLocaleString(locale, 'f', 2) + "ms";
            }

            valueFromText: function(text, locale) {
                return Number.fromLocaleString(locale, text.replace("ms", "")) * 48;
            }
        }

        CheckBox {
            text: qsTr("polarity")
            checked: dataObject.polarity
            onCheckStateChanged: dataObject.polarity = checked
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
            Layout.margins: 5

            onColorChanged: {
                dataObject.color = color
            }
        }

        Component.onCompleted: {
            colorPicker.color = dataObject.color
        }

        ComboBox {
            id: windowSelect
            model: dataObject.windows
            currentIndex: dataObject.window
            onCurrentIndexChanged: dataObject.window = currentIndex
        }

        ComboBox {
            id: powerSelect
            model: [14, 15, 16]
            currentIndex: { model.indexOf(dataObject.fftPower) }
            onCurrentIndexChanged: dataObject.fftPower = model[currentIndex]
        }

        Button {
            text: qsTr("Store");
            onClicked: {
                var storedData = dataObject.store();
                var component = Qt.createComponent("Stored.qml");
                var item = component.createObject(applicationWindow, {dataModel: storedData});

                applicationWindow.dataSourceList.append(item);
            }
        }
    }

    RowLayout {
        spacing: 0
        anchors.left: parent.left
        anchors.right: parent.right

        ComboBox {
            id: deviceSelect
            implicitWidth: 200
            model: dataObject.devices
            currentIndex: { model.indexOf(dataObject.device) }
            onCurrentIndexChanged: dataObject.device = model[currentIndex]
        }

//        CheckBox {
//            text: qsTr("doubleTW")
//            checked: dataObject.doubleTW
//            onCheckStateChanged: dataObject.doubleTW = checked
//        }
    }
    }
}
