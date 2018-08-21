import QtQuick 2.0
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

ProgressBar {
    id:meter
    implicitWidth: parent.width
    property real dBV
    property real yellowLevel : 0.75    //-22dB
    property real redLevel : 0.86       //-12dB
    value: (dBV + 90) / 90              //-90dB ... 0dB

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
          }
}
