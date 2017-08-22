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

    ObjectModel {
          id: sideModel
          Generator {}
          Measure {
              dataModel: measureModel
          }
      }

      ListView {
          id: sideList
          anchors.fill: parent
          anchors.margins: 5
          model: sideModel

          ScrollIndicator.vertical: ScrollIndicator { }
      }

      function append(item) {
          sideModel.append(item);
          modelAdded(item);
      }
}
