import QtQuick 2.15

Rectangle{
    color:"#00aaff"
    anchors.fill: parent
    AnimatedImage{
        id:pumpAnimation
        width:240
        height:160
        smooth:true
        playing: true
        fillMode: Image.PreserveAspectFit
        speed: KrakenZDriver.pumpDuty > 50 ? KrakenZDriver.pumpDuty/100 *6 + 1 : (KrakenZDriver.pumpDuty/100 * 2) + .15
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -36
        anchors.verticalCenter: parent.verticalCenter
        anchors.verticalCenterOffset:0
        source:"pump.gif"
    }



    Rectangle {
        id:puddle
        anchors.fill: parent
        color:"#cc011f64"
    }
    Text{
        color: "#dbfeff"
        anchors.bottom:pumpAnimation.top
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignBottom
        anchors.horizontalCenter: pumpAnimation.horizontalCenter
        anchors.horizontalCenterOffset: 16
        anchors.bottomMargin: 4
        font.pixelSize: 42
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
        width: 76
        height: 212
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
            speed: (KrakenZDriver.fanDuty/100 * 6)
            height:width
            anchors{
                left:parent.left
                right:parent.right
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
            height:width
            anchors{
                left:parent.left
                right:parent.right
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
            height:width
            anchors{
                left:parent.left
                right:parent.right
                top:midFan.bottom
                bottom:parent.bottom
                margins:8
            }
            source:"fan2.gif"
        }
    }
    Rectangle{
        anchors.left:fanBox.left
        width:5
        anchors.leftMargin: -1;
        height: (KrakenZDriver.fanDuty * 204) / 100
        anchors.bottom:fanBox.bottom
        anchors.bottomMargin: 4
        color: "#00f521"
        radius:1
    }
}
