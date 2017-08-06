import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {

    ColumnLayout{
         spacing: 0

         anchors.fill: parent

         Repeater {
             model: 1
             Chart {
                 Layout.fillWidth: true
                 Layout.fillHeight: true
             }
         }
     }

}
