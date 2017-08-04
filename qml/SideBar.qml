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

          Rectangle { height: 100; width: 80; color: "red" }
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
