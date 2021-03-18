import QtQuick 2.15
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

MenuBar {
    id: menuBar;
    style: MenuBarStyle{
                background: Rectangle{ color:Material.backgroundColor}
                itemDelegate: Rectangle {
                    implicitWidth: menuBarLabel.contentWidth * 1.4
                    implicitHeight: menuBarLabel.contentHeight * 1.5
                    color: styleData.selected || styleData.open ? accentColor : backgroundColor
                    Label {
                        id:menuBarLabel
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: menuBarLabel.contentHeight/5
                        color: styleData.selected  || styleData.open ? backgroundColor : foregroundColor
                        text: formatMnemonic(styleData.text,true)
                    }
                }
                menuStyle: MenuStyle {
                    frame: Rectangle {
                        color: backgroundColor
                        border.width: 0
                    }

                    //FIXME: Colors doesn't set correctly from Material
                    itemDelegate {
                        background: Rectangle {
                            color:  styleData.selected || styleData.open ? accentColor : backgroundColor
                            border.width: 0
                        }
                        label: Label {
                            color: styleData.selected ? backgroundColor : foregroundColor
                            text: formatMnemonic(styleData.text,true)
                        }

                        submenuIndicator: Text {
                            text: "\u25ba"
                            color: styleData.selected  || styleData.open ? accentColor : backgroundColor
                        }

                        shortcut: Label {
                            color: styleData.selected ? backgroundColor : foregroundColor
                            text: styleData.shortcut
                        }

                        checkmarkIndicator: CheckBox {
                            checked: styleData.checked
                        }
                    }
                }
            }

        Menu {
            title: qsTr("&File")
            MenuItem {
                text: qsTr("&New")
                shortcut: StandardKey.New
                onTriggered: {
                    applicationWindow.properiesbar.clear();
                    sourceList.reset();
                }
            }
            MenuItem {
                text: qsTr("&Save")
                shortcut: StandardKey.Save
                onTriggered: saveDialog.open();
            }
            MenuItem {
                text: qsTr("&Open")
                shortcut: StandardKey.Open
                onTriggered: openDialog.open()
            }
            MenuItem {
                text: qsTr("&Import")
                shortcut: "Ctrl+I"
                onTriggered: importDialog.open()
            }
            MenuItem {
                text: qsTr("&Add measurement")
                shortcut: "Ctrl+A"
                onTriggered: sourceList.addMeasurement();
            }
            MenuItem {
                text: qsTr("&Add math source")
                shortcut: "Ctrl+M"
                onTriggered: sourceList.addUnion();
            }
            MenuItem {
                text: qsTr("&Add elc")
                shortcut: "Ctrl+L"
                onTriggered: sourceList.addElc();
            }
        }
        Menu {
            title: qsTr("&View")
            MenuItem {
                id: darkModeSelect
                text: qsTr("&Dark Mode")
                shortcut: "Ctrl+D"
                checkable: true
                checked: applicationAppearance.darkMode
                onCheckedChanged: {
                    applicationAppearance.darkMode = darkModeSelect.checked;
                }
            }
            MenuItem {
                id: calculator
                text: qsTr("&Calculator")
                shortcut: "Ctrl+W"
                checkable: false
                onTriggered: {
                    applicationWindow.properiesbar.open(null, "qrc:/Calculator.qml");
                }
            }
        }

        Menu {
            title: qsTr("&Help")
            MenuItem {
                text: qsTr("&Shortcuts")
                shortcut: "F1"
                checkable: false
                onTriggered: shortcutsPopup.open();
            }
            MenuItem {
                text: qsTr("About")
                onTriggered: aboutpopup.open();
                shortcut: "F2"
            }
            MenuItem {
                text: qsTr("Check for update")
                shortcut: "F3"
                onTriggered: update.show();
            }
        }
    }
