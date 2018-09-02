/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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

                ToolTip.visible: hovered
                ToolTip.text: qsTr("average count")
            }
            CheckBox {
                text: qsTr("LPF")
                implicitWidth: 120
                checked: dataObject.lpf
                onCheckStateChanged: dataObject.lpf = checked

                ToolTip.visible: hovered
                ToolTip.text: qsTr("data low pass filter")
            }

            CheckBox {
                text: qsTr("polarity")
                implicitWidth: 120
                checked: dataObject.polarity
                onCheckStateChanged: dataObject.polarity = checked

                ToolTip.visible: hovered
                ToolTip.text: qsTr("inverse polarity at measurement chanel")
            }

            TextField {
                placeholderText: qsTr("title")
                text: dataObject.name
                onTextEdited: dataObject.name = text
                implicitWidth: 120
                ToolTip.visible: hovered
                ToolTip.text: qsTr("title")
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
                ToolTip.visible: hovered
                ToolTip.text: qsTr("series color")
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
                ToolTip.text: qsTr("estimated delay time: <b>%L1ms</b>")
                    .arg(Number(dataObject.estimated / 48).toLocaleString(locale, 'f', 2));
            }

            Button {
                text: qsTr("E");
                onClicked: {
                    delaySpin.value = dataObject.estimated;
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("apply estimated delay")
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
                ToolTip.visible: hovered
                ToolTip.text: qsTr("FFT Size: power of 2")
            }

            ComboBox {
                id: windowSelect
                model: dataObject.windows
                currentIndex: dataObject.window
                onCurrentIndexChanged: dataObject.window = currentIndex
                ToolTip.visible: hovered
                ToolTip.text: qsTr("window function")
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
                ToolTip.visible: hovered
                ToolTip.text: qsTr("measurement chanel number")
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
                ToolTip.visible: hovered
                ToolTip.text: qsTr("reference chanel number")
            }

            ComboBox {
                id: deviceSelect
                Layout.fillWidth: true
                model: dataObject.devices
                currentIndex: { model.indexOf(dataObject.device) }
                onCurrentIndexChanged: dataObject.device = model[currentIndex]
                ToolTip.visible: hovered
                ToolTip.text: qsTr("audio input device")
                displayText: (model.indexOf(dataObject.device) !== -1 ? model.indexOf(dataObject.device) : dataObject.device);
            }

            Button {
                text: qsTr("Store");
                onClicked: {
                    var stored = dataObject.store();
                    stored.name = 'Stored #' + (applicationWindow.dataSourceList.list.model.count - 1);
                    applicationWindow.dataSourceList.addStored(stored);
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("store current measurement")
            }
        }
    }//ColumnLayout
}
