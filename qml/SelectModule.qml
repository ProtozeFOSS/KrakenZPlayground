import QtQuick 2.15

Rectangle {
    color: "#80000000"
    Rectangle{
        anchors.fill: parent
        anchors.topMargin: 20
        anchors.bottomMargin: 20
        color:"#2a2e31"
        Text{
            id:titleText
            text:"Select Module"
            color:"white"
            leftPadding:8
            topPadding:4
            font.pixelSize: 20
            verticalAlignment: Text.AlignVCenter
            font.bold: true
            anchors{
                top:parent.top
                left:parent.left
                right:parent.right
            }
        }

        Rectangle{
            id:addIcon
            width:40
            height:40
            radius:40
            color: "#61d759"
            border.width: 1
            anchors{
                top:parent.top
                right:parent.right
                topMargin:4
            }
            Text{
                anchors.fill: parent
                text:"+"
                font.pixelSize: 28
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
