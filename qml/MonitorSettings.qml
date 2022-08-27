import QtQuick 2.15
import QtQuick.Controls 2.15
Rectangle {
    id:monitorDialog
    color:"#2a2e31"
    signal closeSettings()
    Image{
        id: backButton
        height:48
        width: 48
        anchors{
            left:parent.left
            top:parent.top
            margins:4
        }
        source:"qrc:/images/play.svg"
        rotation:180
        MouseArea{
            anchors.fill: parent
            height:48
            width: 48
            onClicked: monitorDialog.closeSettings()
        }
    }

    Text{
        id: sensorTitle
        anchors{
            top:parent.top
            left:backButton.right
            margins:4
            topMargin:10
        }
        text:"Map System Sensors"
        color:"white"
        font.pixelSize: 28
    }
    Text{
        id:filterTitle
        anchors{
            right:parent.right
            top:parent.top
            left:sensorTitle.right
            margins:1
        }
        text:"Filter"
        color:"white"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.bold: true
        font.pixelSize: 10
    }

    ComboBox{
        textRole:"name"
        anchors{
            right:parent.right
            top:filterTitle.bottom
            left:sensorTitle.right
            margins:2
            rightMargin:6
            leftMargin:12
        }
        height: 36
        model:SystemMonitor.sensorValueTypes()
    }
    ListView {
        id: itemList
        clip:true
        anchors{
            top:sensorTitle.bottom
            left: parent.left
            right:parent.right
            bottom: parent.bottom
            topMargin:12
            leftMargin:4
        }
        delegate: DeviceDelegate{
            width:itemList.width
        }
    }
    Connections{
        target:SystemMonitor
        function onAvailableDevices(devices) {
            itemList.model = devices;
        }
    }

    Component.onCompleted: {
        SystemMonitor.queryDevicesAvailable();
    }
}
