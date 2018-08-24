import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.1
import QtQml 2.2
import QtQml.Models 2.3
import QtQuick.Controls.Material 2.1
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
    property int colorIndex: 6;
    signal modelAdded(Item item);
    signal modelRemoved(Item item);

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
          Measurement {
              dataModel: measurementModel
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
          cacheBuffer: 500

          ScrollIndicator.vertical: ScrollIndicator { }
      }

      function addStored(storedData) {
          var component = Qt.createComponent("Stored.qml");
          var item = component.createObject(applicationWindow, {dataModel: storedData});
          storedData.color = nextColor();

          sideModel.append(item);
          modelAdded(item);
      }

      function remove(object) {
          for (var i = 0; i < sideModel.count; i++) {
              var o = sideModel.get(i);
              if (o.dataModel && o.dataModel === object) {
                  sideModel.remove(i);  //remove from sidebar
                  o.destroy();          //destroy sidebar qml element
                  modelRemoved(o);      //signal for remove from charts and delete Series QObject
                  object.destroy();     //delete Source QObject
                  break;
              }
          }
      }

      function nextColor() {
          var color = Material.color(colorIndex);
          colorIndex += 3;
          if (color === Material.color(-1)) {
              colorIndex -= 20;
              return nextColor();
          }
          return color;
      }
}
