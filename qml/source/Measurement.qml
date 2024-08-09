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
    property var dataModelData : dataModel.data
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
            checked: (dataModelData ? dataModelData.active : false)
            checkedColor: (dataModelData ? dataModelData.color : "none")

            onCheckStateChanged: {
                if (dataModel && dataModelData)
                    dataModelData.active = checked
            }
            error: (dataModelData ? dataModelData.error : false)
        }

        ColumnLayout {
            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                font.bold: highlight
                text:  (dataModelData ? dataModelData.name : "")
            }

            Meter {
                dBV: (dataModelData ? dataModelData.level : 0)
                peak:(dataModelData ? dataModelData.measurementPeak : 0)
                width: parent.width
            }

            Meter {
                dBV: (dataModelData ? dataModelData.referenceLevel : 0)
                peak:(dataModelData ? dataModelData.referencePeak : 0)
                width: parent.width
            }
        }

        Connections {
            target: dataModelData
            function onColorChanged() {
                checkbox.checkedColor = dataModelData.color;
            }
        }

        Component.onCompleted: {
            if (!dataModel || !dataModelData) {
                return;
            }
            if (!dataModelData.isColorValid()) {
                dataModelData.color = sourceList.nextColor();
            }
            dataModelData.errorChanged.connect(function(error) {
                if (error) {
                    applicationWindow.message.showError(qsTr("Can't start the %1.<br/>Device is not supported or busy.").arg(dataModelData.name));
                }
            });
        }
    }
}
