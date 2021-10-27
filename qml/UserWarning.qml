import QtQuick 2.15

Rectangle {
    border.color: "black"
    border.width: 1
    radius: 4
    color: "#94bafd"
    id:userWarning
    signal accepted();
    Loader{
        id: messageLoader
        active: false
        anchors.fill: parent
    }
    Component{
        id: deviceNotfound
        Rectangle{
            color:"grey"
            Image{
                id:sadAF
                anchors.fill: parent
                anchors.margins: 16
                anchors.bottomMargin:window.height/2
                fillMode: Image.PreserveAspectFit
                source: "qrc:/images/SADaf.png"
            }
            Text{
                anchors{
                    top:sadAF.bottom
                    left:parent.left
                    right:parent.right
                    topMargin: 12
                }
                font.pixelSize: 32
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                text:"Sorry no USB device found matching\n VID: 0x17e1  PID: 0x3008\n\nMaybe check the USB connection or drivers?"
            }
        }
    }

    Component{
        id: userWarningMessage
        Rectangle{
            Rectangle {
                color: "#293351"
                radius:8
                anchors.margins:4
                anchors.fill: parent
                Text{
                    wrapMode: Text.WordWrap
                    color:"white"
                    anchors{
                        left:parent.left
                        top:parent.top
                        right:parent.right
                        topMargin:24
                    }
                    font.pixelSize: 24
                    horizontalAlignment: Text.AlignHCenter
                    text:"Use this software under your own risk. Deny or close the application if you do not accept responsibility. Every effort has been made to make this software safe for you to use with your Kraken Z AIO. Please close the CAM software if it is open before continuing."
                }
                Text{
                    wrapMode: Text.WordWrap
                    color:"white"
                    anchors{
                        left:parent.left
                        top:parent.top
                        right:parent.right
                        topMargin:24
                    }
                    font.pixelSize: 24
                    horizontalAlignment: Text.AlignHCenter
                    text:"No attempt to connect will even be made until this dialog is accepted. Deny or close the application if you do not accept responsibility. Every effort has been made to make this software safe for you to use with your Kraken Z AIO. Please close the CAM software if it is open before continuing."
                }
            }
            Image{
                // peyton dog image

            }
        }
    }
    Rectangle{
        id:acceptButton
        anchors.right:parent.right
        anchors.bottom:parent.bottom
        width:140
        height:64
        border.color:"black"
        border.width: 2
        radius:4
        color:"green"
        visible: KrakenZDriver.found && !KrakenZDriver.closeVenderSoftware
        Text{
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color:"white"
            text:"Accept"
            font.pixelSize: 24
        }
        MouseArea{
            anchors.fill: parent
            onClicked:userWarning.accepted();
        }
    }
    Rectangle{
        id:closeButton
        anchors.right:parent.right
        anchors.bottom:parent.bottom
        width:140
        height:64
        border.color:"black"
        border.width: 2
        radius:4
        color:"red"
        Text{
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color:"white"
            text:"Exit"
            font.pixelSize: 24
        }
        MouseArea{
            anchors.fill: parent
            onClicked:Qt.quit()
        }
    }
    Component{
        id:closeCamSoftware
        Rectangle{

        }
    }

    Component.onCompleted: {
        if(!KrakenZDriver.found){
            messageLoader.sourceComponent = deviceNotfound;
        }else if(KrakenZDriver.closeVenderSoftware){
            messageLoader.sourceComponent = closeCamSoftware;
        }else{
            messageLoader.sourceComponent = userWarningMessage;
        }
        messageLoader.active = true;
    }
}
