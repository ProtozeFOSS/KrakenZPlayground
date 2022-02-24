import QtQuick 2.15
import QtGraphicalEffects 1.15
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
            Rectangle {
                radius:8
                anchors.margins:4
                anchors.fill: parent
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
    }

    Component{
        id: userWarningMessage
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
                id:secondClause
                wrapMode: Text.WordWrap
                color:"white"
                anchors{
                    left:parent.left
                    bottom:parent.bottom
                    right:parent.right
                    bottomMargin:76
                }
                font.pixelSize: 24
                horizontalAlignment: Text.AlignHCenter
                text:"For use with Kraken Z3 AIO(s).\nNo connection will be attempted until you accept."
            }
        }

    }

    Component{
        id:closeCamSoftware
        Rectangle{
            Rectangle {
                radius:8
                anchors.margins:4
                anchors.fill: parent
                color:"#666565"
                Image{
                    id:sadAF
                    anchors.fill: parent
                    anchors.margins: 16
                    anchors.bottomMargin:window.height/2
                    fillMode: Image.PreserveAspectFit
                    smooth:true
                    source: "qrc:/images/Icon_no_permission.png"
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
                    color:"white"
                    text:"NZXT KRAKEN Z Device Found!\nBut you don't have access!\n\n Most likely CAM is open or some other sofware is using the LCD endpoint.\nClose it and restart KrakenZPlayground."
                }
            }
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

    Component.onCompleted: {
//        if(!KrakenZDriver.found){
//            messageLoader.sourceComponent = deviceNotfound;
//        }else if(KrakenZDriver.closeVenderSoftware){
//            messageLoader.sourceComponent = closeCamSoftware;
//        }else{
            messageLoader.sourceComponent = userWarningMessage;
//        }
        messageLoader.active = true;
    }
}
