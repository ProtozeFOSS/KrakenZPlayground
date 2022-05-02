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

        radius:8
        anchors.margins:4
        anchors.fill: parent
        color:"grey"
        Text{
            id:titleText
            font.pixelSize: 36
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text:SettingsStatus.message
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
            id:errorText
            anchors{
                top:sadAF.bottom
                left:parent.left
                right:parent.right
                topMargin: 12
            }
            font.pixelSize: 24
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text:SettingsStatus.errorMessage
        }
        Text{
            id:filePath
            anchors{
                top:errorText.bottom
                horizontalCenter: parent.horizontalCenter
                topMargin: 12
            }
            font.pixelSize: 13
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text:SettingsStatus.filePath
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
