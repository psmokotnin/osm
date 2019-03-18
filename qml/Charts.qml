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
import QtQuick.Controls 1.4 //SplitView needs exactly 1.4
import QtQuick.Layouts 1.3

Item {
    id: chartsLayout
    property int count: 1
    property alias firstChart: first
    property alias secondChart: second
    property alias thirdChart: third

    onCountChanged: {
        second.visible = count > 1
        third.visible = count > 2
        first.height = second.height = third.height = chartsLayout.height / count;
    }

    SplitView {
        id: sv
        anchors.fill: parent
        orientation: Qt.Vertical
        readonly property int minimunHeight: 150

         ChartContainer {
             id: first
             Layout.fillWidth: true
             Layout.minimumHeight: sv.minimunHeight
             Layout.preferredWidth: parent.width
             Layout.fillHeight: true
         }

         ChartContainer {
             id: second
             visible: false
             Layout.fillWidth: true
             Layout.minimumHeight: sv.minimunHeight
             Layout.preferredWidth: parent.width
         }

         ChartContainer {
             id: third
             visible: false
             Layout.fillWidth: true
             Layout.minimumHeight: sv.minimunHeight
             Layout.preferredWidth: parent.width
         }
     }

}
