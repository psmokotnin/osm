import QtQuick 2.7
import QtQuick.Controls 1.4 //SplitView needs exactly 1.4
import QtQuick.Layouts 1.3

Item {
    id: chartsLayout
    property int count: 1
    property alias secondVisible: second.visible
    property alias thirdVisible: third.visible

    onCountChanged: {
        secondVisible = count > 1
        thirdVisible = count > 2

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
