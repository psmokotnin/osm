/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.1
import "../" as Root
import OpenSoundMeter 1.0;

Rectangle {
    id: control
    property MeterPlot dataSource
    property var gridRef
    border.color: "grey"
    color: applicationWindow.backgroundColor

    ColumnLayout {
        id: meter
        anchors.fill: parent

        property int heightPart : height / 4
        spacing: 0

        Label {
            Layout.fillWidth: true
            Layout.preferredHeight: meter.heightPart

            text: dataSource ? dataSource.title : "";
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignBottom
        }

        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true

            color:
                dataSource && (dataSource.value > dataSource.threshold) ?
                    Material.color(Material.Red) :
                    Material.color(Material.LightGreen)

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            text: dataSource ? dataSource.value : ""

            font.pixelSize: {(2 * height < parent.width ? meter.heightPart * 2 : parent.width / 3)}
            font.capitalization: Font.AllUppercase
        }

        Label {
            Layout.fillWidth: true
            Layout.preferredHeight: meter.heightPart

            text: dataSource ? dataSource.sourceName : ""
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Root.PropertiesOpener {
       propertiesQml: "qrc:/SPL/MeterProperties.qml"
       cursorShape: Qt.PointingHandCursor
       pushObject: {"meter": control.dataSource, "grid": gridRef}
       onClicked: {
           open();
       }
    }
}
