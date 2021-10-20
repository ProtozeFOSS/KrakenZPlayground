import QtQuick 2.15

Rectangle {
    border.color: "black"
    border.width: 1
    radius: 4
    color: "#94bafd"
    id:userWarning
    signal accepted();
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
            text:"Use this software under your own risk. No attempt to connect will even be made until this dialog is accepted. Deny or close the application if you do not accept responsibility. Every effort has been made to make this software safe for you to use with your Kraken Z AIO. Please close the CAM software if it is open before continuing."
        }
    }


    Image{
        // peyton dog image

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
}
