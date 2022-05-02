import QtQuick 2.15
import QtQuick.Window 2.15

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
        id: missinProfileTop
        radius:8
        anchors.margins:4
        anchors.fill: parent
        color:"grey"
        Text{
            id:titleText
            font.pixelSize: 36
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color:"white"
            text:"Missing profile: "
            anchors{
                top:parent.top
                horizontalCenter:parent.horizontalCenter
                horizontalCenterOffset: -86
                topMargin: 18
            }
        }
        Text {
            anchors.left: titleText.right
            anchors.verticalCenter: titleText.verticalCenter
            font.pixelSize: 36
            color:"red"
            text:ProfileName
        }

        Text{
            id:actionText
            font.pixelSize: 36
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text:"Choose a Profile "
            anchors{
                top:titleText.bottom
                horizontalCenter:parent.horizontalCenter
                topMargin: 8
            }
        }
        Rectangle{
            anchors{
                top:actionText.bottom
                topMargin: 4
                horizontalCenter: parent.horizontalCenter
                bottom:parent.bottom
                bottomMargin: 140
            }
            width:parent.width*.9

            radius:8
            border.width: 3
            border.color: "black"

            GridView{
                anchors.fill: parent
            }
        }

    }
}
