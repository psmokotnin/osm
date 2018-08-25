import QtQuick 2.0
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Popup {
    id: popup
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    ColumnLayout {
        anchors.fill: parent
        Text {
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
            Layout.preferredWidth: popup.availableWidth
            Layout.preferredHeight: 150
            textFormat: Text.RichText
            text: qsTr('
                <h1>OSM</h1><br/>
                <a href="https://psmokotnin.github.io/osm/">https://psmokotnin.github.io/osm/</a><br><br/>
                Copyright (C) 2018  Pavel Smokotnin<br/>
                License: GPL v3.0
            ')
            onLinkActivated: Qt.openUrlExternally(link)
        }

        Text {
            horizontalAlignment: Text.AlignLeft
            textFormat: Text.RichText
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredWidth: popup.availableWidth
            wrapMode: Text.Wrap
            text: qsTr(
                "This program comes with ABSOLUTELY NO WARRANTY. " +
                "This is free software, and you are welcome to redistribute it " +
                "under certain conditions.<br/>" +
                '<a href="https://raw.githubusercontent.com/psmokotnin/osm/master/LICENSE">Full text of license</a>'
            )
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
