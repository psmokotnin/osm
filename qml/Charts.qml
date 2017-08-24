import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {

    ColumnLayout {
        id: sv

        readonly property int minimunHeight: 150

         anchors.fill: parent

         ChartContainer {
             Layout.fillWidth: true
             Layout.minimumHeight: sv.minimunHeight
             Layout.preferredWidth: parent.width
             Layout.fillHeight: true
         }

//         ChartContainer {
//             Layout.fillWidth: true
//             Layout.minimumHeight: sv.minimunHeight
//             Layout.preferredWidth: parent.width
//             Layout.fillHeight: true
//             visible: true
//         }
     }

}
