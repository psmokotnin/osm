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
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3
import OpenSoundMeter 1.0
import "elements"

Item {
    id: chartProperties
    property var dataObject

    RowLayout {
        spacing: 0

        TitledCombo {
            title: qsTr("ppo")
            tooltip: qsTr("points per octave")
            model: ["off", 3, 6, 12, 24, 48]
            currentIndex: {
                var ppo = dataObject.pointsPerOctave;
                if (ppo === 0) ppo = "off";
                model.indexOf(ppo);
            }
            onCurrentIndexChanged: {
                var ppo = model[currentIndex];
                dataObject.pointsPerOctave = (ppo === "off" ? 0 : ppo);
            }
        }

        Button {
            font.family: "Osm"
            text: "\ue804"
            implicitWidth: 60
            onClicked: fileDialog.open();
            ToolTip.visible: hovered
            ToolTip.text: qsTr("save chart as an image")
        }
    }

    FileDialog {
        id: fileDialog
        selectExisting: false
        title: "Please choose a file's name"
        folder: (typeof shortcuts !== 'undefined' ? shortcuts.home : Filesystem.StandardFolder.Home)
        defaultSuffix: "png"
        onAccepted: {
            dataObject.grabToImage(function(result) {
                result.saveToFile(dataObject.urlForGrab(fileDialog.fileUrl));
            });
        }
    }
}
