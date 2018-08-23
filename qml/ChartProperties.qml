import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3

Item {
    id: chartProperties
    property var dataObject

    RowLayout {
        spacing: 0

        TitledCombo {
            title: qsTr("ppo")
            model: ["off", 3, 6, 12, 24, 48]
            currentIndex: {
                var ppo = dataObject.pointsPerOctave;
                if (ppo === 0) ppo = "off";
                model.indexOf(ppo);
            }
            onCurrentIndexChanged: {
                var ppo = model[currentIndex];
                dataObject.pointsPerOctave = (ppo === "off" ? 0 : ppo);
            }
        }

        Button {
            text: qsTr("Save Image");
            onClicked: fileDialog.open();
        }
    }

    FileDialog {
        id: fileDialog
        selectExisting: false
        title: "Please choose a file's name"
        folder: shortcuts.home
        defaultSuffix: "png"
        onAccepted: {
            dataObject.grabToImage(function(result) {
                result.saveToFile(dataObject.urlForGrab(fileDialog.fileUrl));
            });
        }
    }
}
