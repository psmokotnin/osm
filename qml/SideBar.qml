import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.1
/**
 * SideBar
 *
 * There are all current active measurements, stores and sound sources
 * at the sidebar item.
 *
 * For detail settings the bottom bar is used.
 */
Item {

    VisualItemModel {
          id: sideModel
          Generator {}
          Measure {
              m: measureModel
          }

          Rectangle { height: 50; width: parent.width; color: "red" }
          Rectangle { height: 100; width: 80; color: "green" }
          Rectangle { height: 100; width: 80; color: "blue" }
      }

      ListView {
          id: sideList
          anchors.fill: parent
          model: sideModel

          ScrollIndicator.vertical: ScrollIndicator { }
      }
}
