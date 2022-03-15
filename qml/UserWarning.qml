import QtQuick 2.15
import QtGraphicalEffects 1.15
Rectangle {
    border.color: "black"
    border.width: 1
    radius: 4
    color: "#94bafd"
    id:userWarning
    signal accepted();

    Rectangle{
        color: "#293351"
        radius:8
        anchors.margins:4
        anchors.fill: parent
        Text{
            id:firstClause
            wrapMode: Text.WordWrap
            color:"yellow"
            anchors{
                left:parent.left
                top:parent.top
                right:parent.right
                topMargin:32
            }
            font.pixelSize: 28
            horizontalAlignment: Text.AlignHCenter
            text:"USE THIS SOFTWARE AT YOUR OWN RISK!"
        }


        Image{
            id:peytonImg
            source: "qrc:/images/Peyton.png"
            height: 300
            width:300
            visible:false
            anchors{
                right:parent.right
                rightMargin:10
                top:firstClause.bottom
                topMargin:48
            }
        }
        Rectangle{
            id:sourceRect
            width:310
            height:310
            radius:12
            border.width:4
            anchors{
                right:parent.right
                rightMargin:8
                top:firstClause.bottom
                topMargin:48
            }

        }

        OpacityMask{
            maskSource:sourceRect
            source:peytonImg
            anchors{
                right:parent.right
                rightMargin:10
                top:firstClause.bottom
                topMargin:50
            }
            width:306
            height:306
        }
        Text{
            id:dedicationText
            color:"white"
            font.pixelSize: 32
            text:"DEDICATED TO:"
            font.family: "Cambria"
            horizontalAlignment: Text.AlignHCenter
            anchors{
                left:parent.left
                right:sourceRect.left
                rightMargin: 2
                top:sourceRect.top
                topMargin:24
            }
        }
        Text{
            id:nameText
            color: "#0bc0f7"
            style:Text.Sunken
            styleColor: "black"
            text: "PEYTON HOOVER DEAN"
            wrapMode: Text.WordWrap
            font.family: "Lucida Handwriting"
            font.weight: Font.ExtraBold
            font.pixelSize: 48
            anchors.left:dedicationText.left
            anchors.right:dedicationText.right
            anchors.top:dedicationText.bottom
            horizontalAlignment: Text.AlignHCenter
            anchors.topMargin: 16
        }

        Text{
            id:dateText
            text: "2008 - 2021"
            color:"white"
            anchors.left:nameText.left
            anchors.right:nameText.right
            anchors.top:nameText.bottom
            horizontalAlignment: Text.AlignHCenter
            anchors.topMargin: 4
            font.pixelSize:16
        }
        Text{
            id: tributeText
            text:'Shine so bright you become enemy to the Dark\nRest peacefully for I now carry your candle'
            font.family: "Magneto"
            font.pixelSize: 20
            wrapMode: Text.WordWrap
            font.bold: true
            color:"white"
            anchors{
                left:parent.left
                right:parent.right
                top:sourceRect.bottom
                topMargin: 4
            }
            horizontalAlignment: Text.AlignHCenter
        }
        Text{
            id:licenseClause
            wrapMode: Text.WordWrap
            color:"#91e0ff"
            anchors{
                left:parent.left
                bottom:parent.bottom
                right:parent.right
                bottomMargin:150
            }
            font.pixelSize: 28
            style:Text.Sunken
            horizontalAlignment: Text.AlignHCenter
            text:"Developed by Christopher Dean\nReleased under MIT License"
            font.family: "Cambria"
        }
        Text{
            id:secondClause
            color:"white"
            wrapMode: Text.WordWrap
            anchors{
                left:parent.left
                bottom:parent.bottom
                right:parent.right
                bottomMargin:76
            }
            font.pixelSize: 20
            horizontalAlignment: Text.AlignHCenter
            text:"For use with Kraken Z3 AIO(s).\nNo connection will be attempted until you accept."
        }
    }
    Row{
        id: buttonRow
        anchors{
            bottom:parent.bottom
            horizontalCenter: parent.horizontalCenter
            bottomMargin: 8
        }
        height: 64
        spacing: 64
        Rectangle{
            id:acceptButton
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
        Rectangle{
            id:closeButton
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
    }
}
