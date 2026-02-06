import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    width: Screen.width
    height: Screen.height
    visible: true
    title: qsTr("BMS Application")

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        // Pages
        SwipeView {
            id: pages
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            Page {
                Connections {
                    target: bleManager
                    function isBMSdevice(name) {
                        if ((name === "QN9080_BMS") ||
                            (name === "BMS_MCU"))
                        {
                            return true
                        }
                        return false
                    }
                    function onDeviceFound(address, name) {
                        if (isBMSdevice(name)) {
                            bmsDevices.append({ address: address, name: name })
                        }
                    }
                }
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 20
                    Label {
                        text: "Devices"
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.margins: 12
                    }
                    Button {
                        text: "Scan BMS devices"
                        spacing: 6
                        anchors.margins: 12
                        onClicked: bleManager.startScan()
                    }
                    // BMS devices model
                    ListModel {
                        id: bmsDevices
                    }

                    // Scrollable list
                    ListView {
                        id: bmsList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: bmsDevices
                        clip: true
                        spacing: 6

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        delegate: Rectangle {
                            width: bmsList.width/3
                            height: 56
                            radius: 6
                            color: index % 2 ? "#202020" : "#2a2a2a"

                            Column {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 4

                                Text {
                                    text: name
                                    color: "white"
                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: address
                                    color: "#aaaaaa"
                                    font.pixelSize: 12
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                    Button {
                        text: "Stop Scan"
                        onClicked: {
                            bleManager.stopScan()
                            bmsDevices.clear()
                        }
                    }
                }
            }

            Page {
                Connections {
                    target: bleManager
                    function deviceExists(address) {
                        for (var i = 0; i < bleDevicesModel.count; ++i) {
                            if (bleDevicesModel.get(i).address === address)
                                return true
                        }
                        return false
                    }
                    function onDeviceFound(address, name) {
                        if (!deviceExists(address)) {
                            bleDevicesModel.append({ address: address, name: name })
                        }
                    }
                }
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    // Title
                    Label {
                        text: "Logs"
                        font.pixelSize: 18
                        Layout.alignment: Qt.AlignLeft
                        Layout.leftMargin: 12
                    }

                    // Clear button
                    Button {
                        text: "Clear Log"
                        Layout.alignment: Qt.AlignLeft
                        Layout.leftMargin: 12
                        onClicked: bleDevicesModel.clear()
                    }

                    // BLE devices model
                    ListModel {
                        id: bleDevicesModel
                    }

                    // Scrollable list
                    ListView {
                        id: bleList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: bleDevicesModel
                        clip: true
                        spacing: 6

                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        delegate: Rectangle {
                            width: bleList.width/3
                            height: 56
                            radius: 6
                            color: index % 2 ? "#202020" : "#2a2a2a"

                            Column {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 4

                                Text {
                                    text: name
                                    color: "white"
                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: address
                                    color: "#aaaaaa"
                                    font.pixelSize: 12
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }
            }
        }

        // Page indicator
        PageIndicator {
            id: indicator
            Layout.alignment: Qt.AlignHCenter
            count: pages.count
            currentIndex: pages.currentIndex
        }

        // Tabs at the bottom
        TabBar {
            id: tabBar
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 12

            TabButton {
                text: "BMS Devices"
                implicitWidth: 120

                contentItem: Text {
                    text: parent.text
                    color: parent.checked ? "white" : "lightgray"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.fill: parent
                }

                background: Rectangle {
                    color: parent.checked ? "magenta" : "black"
                }
            }

            TabButton {
                text: "Debug Logs"
                implicitWidth: 120

                contentItem: Text {
                    text: parent.text
                    color: parent.checked ? "white" : "lightgray"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.fill: parent
                }

                background: Rectangle {
                    color: parent.checked ? "magenta" : "black"
                }
            }
        }
    }
}
