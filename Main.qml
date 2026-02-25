import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    width: Screen.width
    height: Screen.height
    visible: true
    title: qsTr("BMS Application")

    property BleConnection bleConnection: null

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        // Pages
        SwipeView {
            id: pages
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            // Page with BMS devices
            Page {
                id: mainPage
                property bool portrait: width < height

                contentItem: GridLayout {
                    anchors.fill: parent
                    rowSpacing: 0
                    columnSpacing: 0

                    flow: mainPage.portrait ? GridLayout.TopToBottom : GridLayout.LeftToRight
                    columns: mainPage.portrait ? 1 : 4
                    rows: mainPage.portrait ? 4 : 1

                    // ================= LEFT PANEL =================
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredWidth: mainPage.portrait ? parent.width : parent.width * 0.2
                        Layout.preferredHeight: mainPage.portrait ? parent.height * 0.25 : parent.height
                        spacing: 20

                        RowLayout {
                            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                            Button {
                                text: "Find BMS"
                                width: 100
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.margins: 8
                                onClicked: bleManager.startScan()
                            }
                            Button {
                                text: "Stop Scan"
                                width: 120
                                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                                Layout.margins: 8
                                onClicked: {
                                    bleManager.stopScan()
                                    bmsDevices.clear()
                                }
                            }
                        }

                        // BLE scan callbacks
                        Connections {
                            target: bleManager

                            function isBMSdevice(name) {
                                return (name === "QN9080_BMS" || name === "BMS_MCU")
                            }

                            function onDeviceFound(address, name, vbatLvl) {
                                if (isBMSdevice(name)) {
                                    bmsDevices.append({ address: address, name: name, vbat: vbatLvl })
                                }
                            }
                        }

                        // ================= MODEL =================
                        ListModel { id: bmsDevices }

                        // ================= LIST =================
                        ListView {
                            id: bmsList
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.preferredWidth: mainPage.portrait ? parent.width : parent.width * 0.3
                            Layout.preferredHeight: mainPage.portrait ? parent.height * 0.35 : parent.height
                            model: bmsDevices
                            clip: true
                            spacing: 6

                            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                            delegate: Rectangle {
                                width: bmsList.width
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

                                    Text {
                                        text: vbat + "%"
                                        color: "green"
                                        font.pixelSize: 12
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }

                    // ================= SEPARATOR =================
                    ToolSeparator {
                        orientation: mainPage.portrait ? Qt.Horizontal : Qt.Vertical
                        Layout.fillWidth: mainPage.portrait
                        Layout.fillHeight: !mainPage.portrait
                        background: blue
                    }

                    // ================= RIGHT PANEL =================
                    ColumnLayout {
                        id: rightPanel
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredWidth: mainPage.portrait ? parent.width : parent.width * 0.55
                        Layout.preferredHeight: mainPage.portrait ? parent.height * 0.4 : parent.height
                        spacing: 20
                        Layout.alignment: Qt.AlignTop

                        RowLayout {
                            Layout.alignment: Qt.AlignTop
                            Row {
                                Layout.alignment: Qt.AlignTop
                                Image {
                                    source: "images/battery.png"
                                    width: 64
                                    height: 64
                                    visible: bleConnection?.isConnected ?? false
                                }
                                ColumnLayout {
                                    spacing: 20
                                    Text {
                                        visible: bleConnection?.isConnected ? true : false
                                        color: "green"
                                        text: "Battery: " + bleConnection.batteryLevel + "%"
                                    }
                                }
                            }
                        }

                        Column {
                            spacing: 10

                            Label {
                                text: "Switches"
                                visible: bleConnection?.isConnected ? true: false
                                font.bold: true
                            }

                            Switch {
                                text: "Discharge";
                                implicitHeight: 36
                                visible: bleConnection?.isConnected ? true: false
                                checked: (bleConnection.swState & 1)
                                onToggled: bleConnection.toggleSwitch(0x01)
                            }
                            Switch {
                                text: "Charge";
                                implicitHeight: 36
                                visible: bleConnection?.isConnected ? true: false
                                checked: (bleConnection.swState & 2)
                                onToggled: bleConnection.toggleSwitch(0x02)
                            }
                        }

                        // Read VBAT
                        RowLayout {
                            Layout.alignment: Qt.AlignTop
                            Button {
                                text: "Connect"
                                width: 70
                                Layout.alignment: Qt.AlignTop
                                onClicked: devicePopup.open()
                            }
                            Button {
                                text: "Disconnect"
                                width: 70
                                Layout.alignment: Qt.AlignTop
                                onClicked: bleConnection.disconnectDevice()
                            }
                            Button {
                                visible: bleConnection?.isConnected ? true: false
                                text: "Read BAT"
                                width: 70
                                Layout.alignment: Qt.AlignTop
                                function readVbat() {
                                    bleConnection.readChar(0x2A19);
                                    bleConnection.readChar(0x2BBA);
                                    bleConnection.readChar(0x2BB1);
                                    bleConnection.readChar(0x2BB2);
                                    bleConnection.readChar(0x2BB3);
                                    bleConnection.readChar(0x2BB4);
                                }
                                onClicked: readVbat()                            }
                            Button {
                                visible: bleConnection?.isConnected ? true: false
                                text: "Read SW"
                                width: 70
                                Layout.alignment: Qt.AlignTop
                                onClicked: bleConnection.readChar(0x9AE2)
                            }
                        }

                        // BLE connection callbacks
                        Connections {
                            target: bleConnection
                            ignoreUnknownSignals: true

                            function onConnectedChanged() {
                                if (!bleConnection)
                                    return

                                if (bleConnection.isConnected) {
                                    console.log("BLE connected")
                                    toast.show("BMS connected")
                                } else {
                                    console.log("BLE disconnected")
                                    toast.show("BMS disconnected")
                                }
                            }
                        }

                        // ================= DEVICE POPUP =================
                        Popup {
                            id: devicePopup
                            width: parent.width * 0.8
                            height: parent.height * 0.7
                            modal: true
                            focus: true
                            anchors.centerIn: parent

                            Rectangle {
                                anchors.fill: parent
                                color: "#303030"
                                radius: 8

                                ListView {
                                    anchors.fill: parent
                                    model: bmsDevices

                                    delegate: Rectangle {
                                        width: devicePopup.width
                                        height: 60
                                        color: "#444"

                                        Text {
                                            anchors.centerIn: parent
                                            text: name + " (" + address + ")"
                                            color: "white"
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {

                                                devicePopup.close()

                                                if (bleConnection) {
                                                    console.log("Destroying previous BLE connection")
                                                    bleConnection.disconnectDevice()
                                                    bleConnection.destroy()
                                                    bleConnection = null
                                                }

                                                bleConnection = Qt.createQmlObject(`
                                                    import BMS
                                                    BleConnection {}
                                                `, rightPanel)

                                                console.log("Created BleConnection for", address)

                                                bleConnection.error.connect((err) => {
                                                    console.log("BLE error:", err)
                                                })

                                                bleConnection.connectToDevice(address, name)
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        // ================= TOAST =================
                        Popup {
                            id: toast
                            x: parent.width/2 - width/2
                            y: parent.height - 80
                            width: 300
                            height: 50
                            modal: false
                            focus: false

                            background: Rectangle {
                                color: "#333"
                                radius: 6
                            }

                            Label {
                                anchors.centerIn: parent
                                text: toast.message
                                color: "brown"
                            }

                            property string message: ""

                            function show(msg) {
                                message = msg
                                open()
                                timer.restart()
                            }

                            Timer {
                                id: timer
                                interval: 4000
                                onTriggered: toast.close()
                            }
                        }
                    }
                }
            }

            // Page with debug information
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
                    function onDeviceFound(address, name, batLvl) {
                        if (!deviceExists(address) &&
                            ((name === "QN9080_BMS") ||
                            (name === "BMS_MCU"))) {
                            bleDevicesModel.append({ address: address, name: name, vbat: batLvl })
                        }
                    }
                }
                Connections {
                    target: bleConnection
                    ignoreUnknownSignals: true

                    function onConnectedChanged() {
                        if (!bleConnection)
                            return
                        if (bleConnection.isConnected) {
                            bleDevicesModel.append({ address: "", name: "BMS connected" })
                        } else {
                            bleDevicesModel.append({ address: "", name: "BMS disconnected" })
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

                                Text {
                                    text: vbat + "%"
                                    color: "green"
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

        // Tabs at the bottom to switch Pages
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
