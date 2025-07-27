/**
 *  OSM
 *  Copyright (C) 2025 Pavel Smokotnin

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
import QtQuick 2.15
import QtQuick.Controls 2.1
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.1

import SourceModel 1.0
import OpenSoundMeter 1.0

Item {
    id: control
    property var dataObject;
    property var plot;

    Repeater {
        model: SourceModel {
            list: dataObject && dataObject.data ? dataObject.data.sourceList : list
        }

        Rectangle {
            id: eqpoint
            property var filterData : model.source
            property point beginDrag
            property bool caught: false
            function updateCoordinates() {
                x = plot.v2x(filterData && filterData.data ? filterData.data.cornerFrequency : 1000) - width/2;
                y = plot.v2y(filterData && filterData.data ? filterData.data.gain : 0) - height/2;
            }
            width: 24
            height: 24
            z: mouseArea.drag.active ||  mouseArea.pressed ? 2 : 1
            Component.onCompleted: {
                updateCoordinates();
            }

            color: applicationWindow.backgroundColor
            border {
                width:1;
                color: (dataObject && dataObject.data ? dataObject.data.color : Material.accent)
            }
            radius: 45

            Drag.active: mouseArea.drag.active
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2

            Connections{
                target: plot
                function onXminChanged() {
                    updateCoordinates();
                }
                function onXmaxChanged() {
                    updateCoordinates();
                }
                function onYminChanged() {
                    updateCoordinates();
                }
                function onYmaxChanged() {
                    updateCoordinates();
                }
                function onWidthChanged() {
                    updateCoordinates();
                }
                function onHeightChanged() {
                    updateCoordinates();
                }
            }

            Connections{
                target: (filterData && filterData.data ? filterData.data : target)
                function onCornerFrequencyChanged() {
                    updateCoordinates();
                }
                function onGainChanged() {
                    updateCoordinates();
                }
            }

            Label {
                anchors.centerIn: parent
                text: index +1
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                drag.target: parent

                drag.maximumX: control.width - eqpoint.width / 2
                drag.minimumX: 0 - eqpoint.width / 2
                drag.maximumY: control.height - eqpoint.height / 2
                drag.minimumY: 0 - eqpoint.height / 2

                onPressed: {
                    eqpoint.beginDrag = Qt.point(eqpoint.x, eqpoint.y);
                    mouseArea.cursorShape = Qt.ClosedHandCursor

                }
                onReleased:{
                    mouseArea.cursorShape = Qt.OpenHandCursor
                }
                onDoubleClicked:{
                    if (filterData && filterData.data) {
                        filterData.data.gain = 0;
                        updateCoordinates();
                        eqpoint.filterData.data.q = 0.7;
                    }

                }
                onPositionChanged: function (event) {
                    if (eqpoint.filterData && eqpoint.filterData.data) {
                        eqpoint.filterData.data.gain             = plot.y2v(eqpoint.y + height/2);
                        eqpoint.filterData.data.cornerFrequency  = plot.x2v(eqpoint.x + width/2);
                    }
                }
                onWheel: function(event) {
                    if (eqpoint.filterData && eqpoint.filterData.data) {
                        eqpoint.filterData.data.q *= event.angleDelta.y > 0 ? 1.1 : 0.9;
                    }
                }
            }
        }
    }

    DropArea {
        anchors.fill: parent
        keys: "xyz"
        onEntered: drag.source.caught = true;
        onExited: drag.source.caught = false;
    }
}

