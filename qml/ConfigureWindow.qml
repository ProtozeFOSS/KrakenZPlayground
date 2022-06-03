import QtQuick 2.15
import QtQuick.Controls 2.15
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

    Rectangle{
        id: configureTop
        color:"#2b2b2b"
        anchors.fill: parent
        signal configured()
        property int brightness: 50
        Text{
            anchors{
                top:parent.top
                left:parent.left
                right:parent.right
                topMargin: 8
            }
            color:"white"
            text:"CONFIGURE DISPLAY"
            font.bold: true
            font.pixelSize: 36
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            style: Text.Sunken
        }

        Rectangle {
            radius:4
            height:parent.height * .7
            width:parent.width * .92
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 58
            border.color: "white"
            color:"#9da9b1"
            border.width: 2
            Text{
                anchors{
                    top:parent.top
                    left:parent.left
                    right:parent.right
                    topMargin:8
                }
                font.pixelSize: 20
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                leftPadding: 8
                style: Text.Sunken
                text:"Set the device orientation"
                font.family: "Constantia"
            }
            // 90
            Rectangle {
                color:krakenWhiteRing.rotation == 270 ? "#b37dc791":"#66596973"
                width: 88
                height:72
                anchors{
                    horizontalCenter: parent.horizontalCenter
                    top:parent.top
                    topMargin:42
                }
                Text{
                    color:"#261b29"
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text:"Top"
                    font.pixelSize: 18
                    style:Text.Raised
                    font.bold: true
                }
                MouseArea{
                    anchors.fill: parent
                    onClicked: {krakenWhiteRing.rotation = 270;}
                }
            }

            Image{
                id: krakenWhiteRing
                anchors{
                    top:parent.top
                    topMargin:154
                    horizontalCenter: parent.horizontalCenter
                }
                source:"qrc:/images/Outer Ring and Hose.png"
                height:230
                width: 278
            }

            // 0
            Rectangle {
                color: krakenWhiteRing.rotation == 0 ? "#b37dc791":"#66596973"
                width: 88
                height:72
                anchors{
                    verticalCenter: krakenWhiteRing.verticalCenter
                    right:parent.right
                    rightMargin:40
                }
                Text{
                    color:"#261b29"
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text:"Right"
                    font.pixelSize: 18
                    style:Text.Raised
                    font.bold: true
                }
                MouseArea{
                    anchors.fill: parent
                    onClicked: krakenWhiteRing.rotation = 0;
                }
            }
            // 270
            Rectangle {
                color:krakenWhiteRing.rotation == 90 ? "#b37dc791":"#66596973"
                width: 88
                height:72
                anchors{
                    horizontalCenter: parent.horizontalCenter
                    top:krakenWhiteRing.bottom
                    topMargin:40
                }
                Text{
                    color:"#261b29"
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text:"Bottom"
                    font.pixelSize:18
                    style:Text.Raised
                    font.bold: true
                }
                MouseArea{
                    anchors.fill: parent
                    onClicked: {krakenWhiteRing.rotation = 90;}
                }
            }
            // 180
            Rectangle {
                color:krakenWhiteRing.rotation == 180 ? "#b37dc791":"#66596973"
                width: 88
                height:72
                anchors{
                    verticalCenter: krakenWhiteRing.verticalCenter
                    left:parent.left
                    leftMargin:40
                }
                Text{
                    color:"#261b29"
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text:"Left"
                    font.pixelSize:18
                    style:Text.Raised
                    font.bold: true
                }
                MouseArea{
                    anchors.fill: parent
                    onClicked: {krakenWhiteRing.rotation = 180;}
                }
            }
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 80
            anchors.bottom:parent.bottom
            width:parent.width *.8
            color:"transparent"
            height: 72
            Text{
                id: setBrightnessLabel
                text:"Set LCD Brightness"
                font.family: "Constantia"
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors{
                    top: parent.top
                    left:parent.left
                    right:parent.right
                    topMargin:2
                }
                font.pixelSize: 24
                color:"white"
                style: Text.Sunken
                styleColor: "#737272"
                font.letterSpacing: 4
                leftPadding: 8
            }

            Text{
                id: zeroBrightnessLabel
                color:"white"
                text:"0%"
                leftPadding:16
                horizontalAlignment: Text.AlignHCenter
                anchors{
                    left: parent.left
                    verticalCenter: setBrightnessSlider.verticalCenter
                }
            }

            Slider{
                id:setBrightnessSlider
                anchors{
                    top:setBrightnessLabel.bottom
                    topMargin:2
                    left:zeroBrightnessLabel.right
                    right:fullBrightnessLabel.left
                }
                snapMode:Slider.SnapAlways
                live:true
                from: 0
                to: 100
                stepSize:1
                value:configureTop.brightness
                handle:Rectangle{
                    color: "#655e71"
                    border.color: "#b9b9b9"
                    border.width: 2
                    height: parent.height * .8
                    width: height
                    radius:width
                    x: setBrightnessSlider.leftPadding + setBrightnessSlider.visualPosition * (setBrightnessSlider.availableWidth - width)
                    y: setBrightnessSlider.topPadding + setBrightnessSlider.availableHeight / 2 - height / 2
                }
                background: Rectangle {
                    x: setBrightnessSlider.leftPadding
                    y: setBrightnessSlider.topPadding + setBrightnessSlider.availableHeight / 2 - height / 2
                    implicitWidth: 200
                    implicitHeight: 16
                    width: setBrightnessSlider.availableWidth
                    height: setBrightnessSlider.height *.35
                    radius: 2
                    color: "#e6e6e6"
                    border.width: 2
                    border.color:"#7e7e7e"

                    Rectangle {
                        width: setBrightnessSlider.visualPosition * parent.width
                        height: parent.height
                        color: "#cc03d429"
                        radius: 2
                    }
                }
                onValueChanged: {
                    configureTop.brightness = value;
                }

                height: 36
            }
            Text{
                id: brightnessValue
                color:"white"
                text:setBrightnessSlider.value + "%"
                font.pixelSize: 16
                font.bold: true
                anchors{
                    horizontalCenter: setBrightnessSlider.horizontalCenter
                    top:setBrightnessSlider.bottom
                    topMargin: 2
                }
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Text{
                id: fullBrightnessLabel
                color:"white"
                text:"100%"
                rightPadding:16
                horizontalAlignment: Text.AlignHCenter
                anchors{
                    verticalCenter: setBrightnessSlider.verticalCenter
                    right:parent.right
                }
            }
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 4
            height: 40
            width: 160
            radius: 4
            border.color: "black"
            border.width: 2
            color:"#274a71"
            Text{
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                text:"Continue"
                color:"white"
                font.bold: true
                font.pixelSize: 20
            }
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    AppController.setOrientationFromAngle(krakenWhiteRing.rotation)
                    DeviceConnection.setBrightness(configureTop.brightness)
                    KZP.configured()
                }
            }
        }
    }

}
