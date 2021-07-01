/**
 *  OSM
 *  Copyright (C) 2019  Pavel Smokotnin

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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3
import "elements"

Item {
    id: calculator
    property var temp : 20
    property double speed : speedOfSound()
    property var frequency: 1000
    property double wave
    property var mode: 0 //0 - meter | 1 - foot
    property var dataObject

    function speedOfSound() {
        return 20.05 * Math.sqrt(273.15 + temp);
    }

    function waveLength() {
        return wave = (mode ? 3.281 : 1) * speed / frequency;
    }

    onTempChanged:     speed = speedOfSound();
    onWaveChanged: frequency = (mode ? 3.281 : 1) * speed / wave;
    onFrequencyChanged: waveLength();
    onSpeedChanged:     waveLength();
    onModeChanged:      waveLength();

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

    RowLayout {
        spacing: 0

        SelectableSpinBox {
            id: frequencySB
            editable: true
            from: 1
            to: 200000
            value: frequency
            onValueModified: frequency = value;
            ToolTip.visible: hovered
            ToolTip.text: qsTr("frequency")

            textFromValue: function(value, locale) {
                return Number(value) + " Hz"
            }

            valueFromText: function(text, locale) {
                return parseInt(text)
            }
        }

        SelectableSpinBox {
            id: time
            property int decimals: 1000
            editable: true
            stepSize: 10
            from:  1000 * decimals / frequencySB.to
            to:    1000 * decimals / frequencySB.from
            value: 1000 * decimals / frequency
            onValueModified: {
                frequency = Math.round(10000 * decimals / value) / 10;
            }
            ToolTip.visible: hovered
            ToolTip.text: qsTr("period")

            textFromValue: function(value, locale) {
                return Number(value / decimals).toLocaleString(locale, 'f', 3) + " ms";
            }

            valueFromText: function(text, locale) {
                return parseFloat(text.replace(',', '.')) * decimals
            }

        }

        Rectangle {
            Layout.fillWidth: true
        }

        SelectableSpinBox {
            value: calculator.temp
            onValueChanged: {
                if (value != calculator.temp) {
                    calculator.temp = value;
                }
            }
            from: -30
            to: 50
            editable: true
            implicitWidth: 170

            ToolTip.visible: hovered
            ToolTip.text: Number(value * 9/5 + 32).toLocaleString(locale, 'f', 0) + "ºF"

            textFromValue: function(value, locale) {
                return Number(value) + "ºC"
            }

            valueFromText: function(text, locale) {
                return parseInt(text)
            }

        }

        SelectableSpinBox {
            id: airSpeed
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: 170
            value: calculator.speed * 10
            from: 0
            to: 5000
            editable: false
            down.indicator.visible: false
            up.indicator.visible:   false
            down.indicator.enabled: false
            up.indicator.enabled:   false

            textFromValue: function(value, locale) {
                return Number(calculator.speed).toLocaleString(locale, 'f', 1) + " m/s";
            }

            ToolTip.visible: hovered
            ToolTip.text: qsTr("speed of sound");
        }
    }
    RowLayout {
        spacing: 0

        SelectableSpinBox {
            id: wavelength
            property int decimals: 1000
            editable: true
            stepSize: 10
            from:  1
            to:    100000 * decimals
            value: wave * decimals
            onValueModified: wave = value / decimals

            ToolTip.visible: hovered
            ToolTip.text: qsTr("wave length")

            textFromValue: function(value, locale) {
                return Number(value / decimals).toLocaleString(locale, 'f', 3) + (mode ? " ft" : " m");
            }

            valueFromText: function(text, locale) {
                return parseFloat(text.replace(',', '.')) * decimals;
            }

        }

        SelectableSpinBox {
            property int decimals: 1000
            editable: true
            stepSize: 1
            from:  1
            to:    100000 * decimals
            value: wave * decimals / 2
            onValueModified: wave = value * 2 / decimals

            ToolTip.visible: hovered
            ToolTip.text: qsTr("wave length / 2")

            textFromValue: function(value, locale) {
                return Number(value / decimals).toLocaleString(locale, 'f', 3) + (mode ? " ft" : " m");
            }

            valueFromText: function(text, locale) {
                return parseFloat(text.replace(',', '.')) * decimals;
            }

        }

        Rectangle {
            Layout.fillWidth: true
        }

        DropDown {
            implicitWidth: 170
            model: ["meter", "foot"]
            currentIndex: mode
            onCurrentIndexChanged: {
                if (mode != currentIndex) {
                    mode = currentIndex
                }
            }
        }
    }
  }
    Component.onCompleted: {
        if (dataObject && dataObject.frequency) {
            frequency = dataObject.frequency;
        } else if (dataObject && dataObject.time) {
            frequency = 1000 / dataObject.time;
        }
    }
}
