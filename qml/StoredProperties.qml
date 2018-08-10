import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2

Item {

    property var dataObject

    RowLayout {
        spacing: 0

        TextField {
            placeholderText: qsTr("title")
            text: dataObject.name
            onTextEdited: dataObject.name = text

        }

        ColorPicker {
            id: colorPicker

            Layout.preferredWidth: 25
            Layout.preferredHeight: 25
            Layout.margins: 5

            onColorChanged: {
                dataObject.color = color
            }
        }

        Button {
            text: qsTr("Delete");
            onClicked: {
                applicationWindow.dataSourceList.remove(dataObject);
                applicationWindow.properiesbar.stack.clear();
            }
        }

        Component.onCompleted: {
            colorPicker.color = dataObject.color
        }
    }
}
