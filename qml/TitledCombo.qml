import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {
    id: i
    width: cb.width + 10

    property string title : ""
    property var model : []
    property int currentIndex : 0
    //signal

    ColumnLayout {

        spacing: 1

        Text {
            Layout.margins: 5
            text: title
        }

        ComboBox {
            id: cb
            Layout.leftMargin: 5
            Layout.bottomMargin: 5
            model: i.model
            currentIndex: i.currentIndex
        }
    }
}
