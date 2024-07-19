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
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import "qrc:/"

Item {
    id: measurement

    property var dataModel;
    property bool chartable : true;
    property bool highlight : false;
    property string propertiesQml: "qrc:/source/MeasurementProperties.qml"
    height: 50
    width: (parent ? parent.width : 0)

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: 10

        MulticolorCheckBox {
            id: checkbox
            Layout.alignment: Qt.AlignVCenter
            checked: (dataModel.data ? dataModel.data.active : false)
            checkedColor: (dataModel.data ? dataModel.data.color : "none")

            onCheckStateChanged: {
                if (dataModel && dataModel.data)
                    dataModel.data.active = checked
            }
            error: (dataModel.data ? dataModel.data.error : false)
        }

        ColumnLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                font.bold: highlight
                text:  (dataModel.data ? dataModel.data.name : "")
            }

            Meter {
                dBV: (dataModel.data ? dataModel.data.level : 0)
                peak:(dataModel.data ? dataModel.data.measurementPeak : 0)
                width: parent.width
            }

            Meter {
                dBV: (dataModel.data ? dataModel.data.referenceLevel : 0)
                peak:(dataModel.data ? dataModel.data.referencePeak : 0)
                width: parent.width
            }
        }

        Connections {
            target: dataModel.data
            function onColorChanged() {
                checkbox.checkedColor = dataModel.data.color;
            }
        }

        Component.onCompleted: {
            if (!dataModel || !dataModel.data) {
                return;
            }
            if (!dataModel.data.isColorValid()) {
                dataModel.data.color = applicationWindow.dataSourceList.nextColor();
            }
            dataModel.data.errorChanged.connect(function(error) {
                if (error) {
                    applicationWindow.message.showError(qsTr("Can't start the %1.<br/>Device is not supported or busy.").arg(dataModel.data.name));
                }
            });
        }
    }
}
