import QtQuick 2.15
import QtQuick.Window 2.15
import com.application.kzp 1.0

Window {
    id:window
    width: 600
    height:720
    visible: true
    minimumHeight: 720
    minimumWidth:600
    maximumHeight:720
    maximumWidth: 600
    Rectangle {
        id: permissionRoot
        color:"#2a2e31"
        property alias errorMessage: error.text
        signal useSoftware()
        anchors.fill: parent
        Rectangle{
            border.color: "black"
            color: "#ffce47"
            radius:16
            anchors {
                horizontalCenter: parent.horizontalCenter
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: -60
            }
            width:parent.width * .8
            height: parent.height * .6
            Text{
                id:title
                anchors {
                    top: parent.top
                    left:permissionIcon.right
                    leftMargin:8
                    topMargin:64
                }
                width:parent.width * .9
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignTop
                color:"red"
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: 42
                style: Text.Sunken
                text: ControllerStatus.type == KZPController.ERROR_DEVICE_NF ? "DEVICE\nNOT FOUND":"PERMISSIONS\nERROR"
            }
            Image{
                id:permissionIcon
                source:ControllerStatus.type == KZPController.ERROR_DEVICE_NF ? "qrc:/images/not-found.png":"qrc:/images/Icon_no_permission.png"
                height:ControllerStatus.type == KZPController.ERROR_DEVICE_NF ? 180:128
                fillMode: Image.PreserveAspectFit
                anchors{
                    top:parent.top
                    left:parent.left
                    leftMargin:32
                    topMargin:42
                }
            }

            Text{
                anchors{
                    top:permissionIcon.bottom
                    left:parent.left
                    right:parent.right
                    topMargin: 4
                }
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                style:Text.Sunken
                color:"green"
                text:  ControllerStatus.type == KZPController.ERROR_DEVICE_NF ? "Kraken Z Device not found\ncheck your cables and installation":"NZXT KRAKEN Z Device Found!\n" + ControllerStatus.errorMessage
            }
            Text{
                id:error
                anchors {
                    verticalCenter: parent.verticalCenter
                    horizontalCenter:parent.horizontalCenter
                    verticalCenterOffset: 72
                }
                width:parent.width * .9
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color:"black"
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: 24
                text:ControllerStatus.message
            }
            Rectangle{
                anchors.bottom: parent.bottom
                anchors.horizontalCenter:  parent.horizontalCenter
                anchors.horizontalCenterOffset: -94
                anchors.bottomMargin: 20
                width:180
                height:80
                radius:8
                border.color: "white"
                color:"green"
                Text{
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color:"white"
                    text:"Use Software\nDriver"
                    font.pixelSize: 24
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: KZP.selectSoftwareDriver();
                }
            }

            Rectangle{
                anchors.bottom: parent.bottom
                anchors.horizontalCenter:  parent.horizontalCenter
                anchors.horizontalCenterOffset: 94
                anchors.bottomMargin: 20
                width:180
                height:80
                radius:8
                border.color: "white"
                color:"red"
                Text{
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color:"white"
                    text:"Close"
                    font.pixelSize: 32
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: Qt.quit();
                }
            }
        }
    }
}

//Rectangle{
//    Rectangle {
//        radius:8
//        anchors.margins:4
//        anchors.fill: parent
//        color:"#666565"
//        Image{
//            id:sadAF
//            anchors.fill: parent
//            anchors.margins: 16
//            anchors.bottomMargin:window.height/2
//            fillMode: Image.PreserveAspectFit
//            smooth:true
//            source: "qrc:/images/Icon_no_permission.png"
//        }
//        Text{
//            anchors{
//                top:sadAF.bottom
//                left:parent.left
//                right:parent.right
//                topMargin: 12
//            }
//            font.pixelSize: 32
//            horizontalAlignment: Text.AlignHCenter
//            verticalAlignment: Text.AlignVCenter
//            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
//            color:"white"
//            text:"NZXT KRAKEN Z Device Found!\nBut you don't have access!\n\n Most likely CAM is open or some other sofware is using the LCD endpoint.\nClose it and restart Kraken Z Playground."
//        }
//    }
//}
