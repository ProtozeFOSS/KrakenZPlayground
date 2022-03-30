import QtQuick 2.15

Rectangle{
    color:"#00aaff"
    anchors.fill: parent
    AnimatedImage{
        id:pumpAnimation
        width:490
        smooth:true
        playing: true
        fillMode: Image.PreserveAspectFit
        speed: ((3 + (KrakenZDriver.fps / PrimaryScreen.refreshRate)) * KrakenZDriver.pumpDuty) / 200
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -1
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset:0
        source:"pump.gif"
    }

    Rectangle{
        id:leftBump
        color:"#6b6b6b"
        border.width: 2
        width:24
        height:24
        radius:4
        border.color: "black"
        Rectangle{
            color: "#6e9cb9"
            border.color: "#ffffff"
            border.width: 1
            radius:2
            anchors.fill: parent
            anchors.margins: 6
        }
        anchors{
            top:lcdFrame.top
            right:lcdFrame.left
            topMargin:8
            rightMargin:-2
        }
    }
    Rectangle{
        id:rightBump
        color:"#6b6b6b"
        border.width: 2
        width:24
        height:24
        radius:4
        border.color: "black"
        Rectangle{
            color: "#6e9cb9"
            border.color: "#ffffff"
            border.width: 1
            radius:2
            anchors.fill: parent
            anchors.margins: 6
        }
        anchors{
            top:lcdFrame.top
            left:lcdFrame.right
            topMargin:8
            leftMargin:-2
        }
    }
    Rectangle{
        height:48
        border.width: 2
        radius:12
        border.color: "#000000"
        width:68
        color: "#fa737373"

        anchors{
            top:lcdFrame.bottom
            topMargin:-26
            horizontalCenter: lcdFrame.horizontalCenter
        }
        Text{
            color: "#92dbf3"
            anchors.fill: parent
            anchors.margins: 2
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignBottom
            text: "TEMP"
            font.family: "Consolas"
            font.pixelSize: 18
            font.bold: true
            style: Text.Sunken
            styleColor: "#437295"
        }
    }
    Rectangle{
        id:lcdFrame
        width:120
        height:44
        radius:8
        border.color: "black"
        border.width:2
        anchors{
            horizontalCenter: parent.horizontalCenter
            top:parent.top
            topMargin:-6
        }

        color:"#605f5f"
        Rectangle{
            id:innerPlate
            color:"#afafaf"
            border.color: "#7a7a7a"
            radius:8
            anchors{
                fill:parent
                margins:2
                topMargin:0
            }
            Text{
                color: "#000dc6"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                anchors.fill: parent
                anchors.margins: 4
                font.pixelSize: 28
                style: Text.Outline
                font.bold: true
                styleColor: "#626262"
                text:KrakenZDriver.liquidTemperature.toString().slice(0,4) + " °C";
                font.family: "Cambria"
            }

        }
    }




    Rectangle{
        id:pumpFrame
        width:120
        height:44
        radius:8
        border.color: "black"
        border.width:2
        anchors{
            horizontalCenter: parent.horizontalCenter
            bottom:parent.bottom
            topMargin:-1
        }

        color:"#605f5f"
        Rectangle{
            id:pumpPlate
            color:"#afafaf"
            border.color: "#7a7a7a"
            radius:8
            anchors{
                fill:parent
                margins:2
                bottomMargin:0
            }
            Text{
               id: pumpRPM
               anchors.fill: parent
               font.pixelSize: 24
               horizontalAlignment: Text.AlignHCenter
               verticalAlignment: Text.AlignBottom
               text:KrakenZDriver.pumpSpeed;
               font.family: "Cambria"
               color:"#e68703"
               font.bold: true
               style:Text.Outline
               styleColor: "black"
            }

        }
    }
    Rectangle{
        height:20
        border.width: 2
        radius:4
        width:88
        color: "#404040"

        anchors{
            bottom:pumpFrame.top
            bottomMargin:-14
            horizontalCenter: pumpFrame.horizontalCenter
        }
        Text{
            color: "#d38806"
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "Pump RPM"
            font.pixelSize: 12
            font.bold: true
            style: Text.Raised
            styleColor: "#000000"
        }
    }

    Rectangle{
        id:lpumpBump
        color:"#6b6b6b"
        border.width: 2
        width:28
        height:28
        radius:8
        rotation:45
        border.color: "black"
        Rectangle{
            color: "#dbdbdb"
            border.width: 1
            anchors.fill: parent
            anchors.margins: 6
        }

        anchors{
            bottom:pumpFrame.bottom
            left:pumpFrame.left
            bottomMargin:-2
            leftMargin:-20
        }
    }
    Rectangle{
        id:rpumpBump
        color:"#6b6b6b"
        border.width: 2
        width:28
        height:28
        radius:8
        rotation:45
        border.color: "black"
        Rectangle{
            color: "#dbdbdb"
            border.width: 1
            anchors.fill: parent
            anchors.margins: 6
        }

        anchors{
            bottom:pumpFrame.bottom
            right:pumpFrame.right
            bottomMargin:-2
            rightMargin:-20
        }
    }
    Rectangle{
        id:fanBox
        width: 88
        height: 308
        color:"#292929"
        border.color: "#4f4f4f"
        radius:6
        border.width: 2
        anchors.right:parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: -12

        AnimatedImage{
            id:topFan
            smooth:true
            playing: true
            fillMode: Image.PreserveAspectFit
            speed: (KrakenZDriver.fanDuty/100) * 1.1
            width:84
            height:width
            anchors{
                horizontalCenter: parent.horizontalCenter
                horizontalCenterOffset: 4
                top:parent.top
                topMargin:20
                margins:6
            }
            source:"fan2.gif"
        }
        AnimatedImage{
            id:midFan
            smooth:true
            playing: true
            fillMode: Image.PreserveAspectFit
            speed: topFan.speed
            width:84
            height:width
            anchors{
                horizontalCenter: parent.horizontalCenter
                horizontalCenterOffset: 4
                top:topFan.bottom
                margins:6
            }
            source:"fan2.gif"
        }
        AnimatedImage{
            smooth:true
            playing: true
            fillMode: Image.PreserveAspectFit
            speed: topFan.speed
            width:84
            height:width
            anchors{
                horizontalCenter: parent.horizontalCenter
                horizontalCenterOffset: 4
                top:midFan.bottom
                margins:6
            }
            source:"fan2.gif"
        }
    }
    Rectangle{
        anchors.left:fanBox.left
        width:4
        anchors.leftMargin: -1;
        height: (KrakenZDriver.fanDuty * 304) / 100
        anchors.bottom:fanBox.bottom
        anchors.bottomMargin: -4
        color: "#00f521"
        radius:1
    }
}
