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

    Text{
        color: "#dbfeff"
        anchors.bottom:pumpAnimation.top
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignBottom
        anchors.horizontalCenter: pumpAnimation.horizontalCenter
        anchors.horizontalCenterOffset: 16
        anchors.bottomMargin: 4
        font.pixelSize: 32
        style: Text.Raised
        font.bold: true
        text:KrakenZDriver.liquidTemperature.toString().slice(0,5) + " °C";
        font.family: "Cambria"
    }
    Text{
       id: pumpRPM
       anchors.horizontalCenter: pumpAnimation.horizontalCenter
       anchors.top: pumpAnimation.bottom
       anchors.bottomMargin: 8
       anchors.horizontalCenterOffset: 16
       font.pixelSize: 32
       text:KrakenZDriver.pumpSpeed + " RPM";
       font.family: "Cambria"
       color:"#e68703"
       style:Text.Sunken
       styleColor: "black"
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
        anchors.rightMargin: -2

        AnimatedImage{
            id:topFan
            smooth:true
            playing: true
            fillMode: Image.PreserveAspectFit
            speed: ((4 + (KrakenZDriver.fps / PrimaryScreen.refreshRate)) * KrakenZDriver.fanDuty) / 400
            width:84
            height:width
            anchors{
                horizontalCenter: parent.horizontalCenter
                horizontalCenterOffset: 4
                top:parent.top
                margins:8
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
                margins:8
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
                margins:8
            }
            source:"fan2.gif"
        }
    }
    Rectangle{
        anchors.left:fanBox.left
        width:5
        anchors.leftMargin: -1;
        height: (KrakenZDriver.fanDuty * 310) / 100
        anchors.bottom:fanBox.bottom
        anchors.bottomMargin: -2
        color: "#00f521"
        radius:1
    }
    Text{
      anchors{
         bottom:parent.bottom
         bottomMargin:4
         horizontalCenter: parent.horizontalCenter
      }
      text:"Bucket:" + KrakenZDriver.bucket
      font.pixelSize: 12
      horizontalAlignment: Text.AlignHCenter
      verticalAlignment: Text.AlignVCenter
      color:"black"
    }
}
