import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.1
import QtQml.Models 2.2
/**
 * SideBar
 *
 * There are all current active measurements, stores and sound sources
 * at the sidebar item.
 *
 * For detail settings the bottom bar is used.
 */
Item {
    property Item list : sideList

    signal modelAdded(Item item);

    ComboBox {
        id: chartsCount
        anchors.horizontalCenter: parent.horizontalCenter
        model: ["Single", "Double", "Three"]
        currentIndex: 0
        onCurrentIndexChanged: {
            applicationWindow.charts.count = currentIndex + 1
        }
    }

    ObjectModel {
          id: sideModel
          Generator {}
          Measure {
              dataModel: measureModel
          }
      }

      ListView {
          id: sideList
          anchors.top: chartsCount.bottom
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.bottom: parent.bottom
          anchors.margins: 5
          model: sideModel

          ScrollIndicator.vertical: ScrollIndicator { }
      }

      function append(item) {
          sideModel.append(item);
          modelAdded(item);
      }
}
