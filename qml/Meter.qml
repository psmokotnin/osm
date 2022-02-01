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
import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

ProgressBar {
    id:meter
    implicitWidth: parent.width
    property real dBV
    property real peak
    property real yellowLevel : 0.75    //-22dB
    property real redLevel : 0.86       //-12dB
    value: (dBV + 90) / 90              //-90dB ... 0dB
    property real peakValue: (peak + 90) / 90

    contentItem: Item {

              Rectangle {
                  id: green
                  width: Math.min(meter.value, yellowLevel) * parent.width
                  height: parent.height
                  radius: 2
                  color: Material.color(Material.Green)
              }
              Rectangle {
                  id: yellow
                  anchors.left: green.right
                  width: (meter.value > yellowLevel ? Math.min(meter.value, redLevel) - yellowLevel : 0) * parent.width
                  height: parent.height
                  radius: 2
                  color: Material.color(Material.Orange)
              }
              Rectangle {
                  id: red
                  anchors.left: yellow.right
                  width: (meter.value > redLevel ? meter.value - redLevel : 0) * parent.width
                  height: parent.height
                  radius: 2
                  color: Material.color(Material.Red)
              }

              Rectangle {
                  id: peakInd
                  anchors.left: green.left
                  anchors.leftMargin: meter.peakValue * parent.width
                  width: 2
                  height: parent.height
                  radius: 2
                  color: (peakValue > redLevel ? Material.color(Material.Red) : (peakValue > yellowLevel ? Material.color(Material.Orange) : Material.color(Material.Green)))
              }
          }
}
