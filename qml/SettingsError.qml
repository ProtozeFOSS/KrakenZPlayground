import QtQuick 2.15

Rectangle{
    id:missingTop
    Rectangle {

        radius:8
        anchors.margins:4
        anchors.fill: parent
        color:"grey"
        Text{
            id:titleText
            font.pixelSize: 32
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text:"Settings File not found\n or it contained errors"
            anchors{
                top:parent.top
                horizontalCenter:parent.horizontalCenter
                topMargin: 32
            }
        }

        Image{
            id:sadAF
            anchors.top:titleText.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 16
            height:parent.height/3
            width:height
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
            font.pixelSize: 24
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text:"Please check your file path \n and the profile name '" + SettingsManager.currentProfile() +  "' exists \n or check for errors syntax"
        }
        Rectangle{
            id:closeButton
            width:140
            height:64
            border.color:"black"
            border.width: 2
            radius:4
            color:"red"
            anchors{
                bottom:parent.bottom
                horizontalCenter:parent.horizontalCenter
                bottomMargin:32
            }

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
    }
}
