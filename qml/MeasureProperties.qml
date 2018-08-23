import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Item {
    property var dataObject

    ColumnLayout {
        spacing: 0
        anchors.fill: parent

    RowLayout {

        SpinBox {
            implicitWidth: 120
            value: dataObject.average
            from: 1
            to: 100
            editable: true
            onValueChanged: dataObject.average = value
        }
        CheckBox {
            text: qsTr("LPF")
            implicitWidth: 120
            checked: dataObject.lpf
            onCheckStateChanged: dataObject.lpf = checked
        }

        CheckBox {
            text: qsTr("polarity")
            implicitWidth: 120
            checked: dataObject.polarity
            onCheckStateChanged: dataObject.polarity = checked
        }

            TextField {
                placeholderText: qsTr("title")
                text: dataObject.name
                onTextEdited: dataObject.name = text
                implicitWidth: 120
            }

            ColorPicker {
                id: colorPicker
                Layout.preferredWidth: 25
                Layout.preferredHeight: 25
                Layout.margins: 0

                onColorChanged: {
                    dataObject.color = color
                }

                Component.onCompleted: {
                    color = dataObject.color
                }
            }

            RowLayout {
                Layout.fillWidth: true
            }

            SpinBox {
                id: delaySpin
                implicitWidth: 180
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

                ToolTip.visible: hovered
                ToolTip.text: "Estimated delay time: <b>" +
                              Number(dataObject.estimated / 48).toLocaleString(locale, 'f', 2) +
                              'ms</b>';
            }

            Button {
                text: qsTr("E");
                onClicked: {
                    delaySpin.value = dataObject.estimated;
                }
            }
    }

    RowLayout {
        Layout.fillWidth: true

        ComboBox {
            id: powerSelect
            model: [14, 15, 16]
            currentIndex: { model.indexOf(dataObject.fftPower) }
            onCurrentIndexChanged: dataObject.fftPower = model[currentIndex]
            displayText: "Power:" + currentText
        }

        ComboBox {
            id: windowSelect
            model: dataObject.windows
            currentIndex: dataObject.window
            onCurrentIndexChanged: dataObject.window = currentIndex
        }

        ComboBox {
            model: dataObject.chanelsCount
            currentIndex: dataObject.dataChanel
            onCurrentIndexChanged: dataObject.dataChanel = currentIndex
            displayText: "M ch:" + (currentIndex + 1)
            delegate: ItemDelegate {
                      text: modelData + 1
                      width: parent.width
                  }
        }

        ComboBox {
            model: dataObject.chanelsCount
            currentIndex: dataObject.referenceChanel
            onCurrentIndexChanged: dataObject.referenceChanel = currentIndex
            displayText: "R ch:" + (currentIndex + 1)
            delegate: ItemDelegate {
                      text: modelData + 1
                      width: parent.width
                  }
        }

        ComboBox {
            id: deviceSelect
            Layout.fillWidth: true
            model: dataObject.devices
            currentIndex: { model.indexOf(dataObject.device) }
            onCurrentIndexChanged: dataObject.device = model[currentIndex]
        }

        Button {
            text: qsTr("Store");
            onClicked: {
                var stored = dataObject.store();
                stored.name = 'Stored #' + (applicationWindow.dataSourceList.list.model.count - 1);
                applicationWindow.dataSourceList.addStored(stored);
            }
        }
    }
    }//ColumnLayout
}
