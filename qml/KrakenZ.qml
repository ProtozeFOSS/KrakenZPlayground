import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Qt.labs.platform 1.1
import OffscreenApp 1.0
import QtGraphicalEffects 1.15
Rectangle {
    id: krakenRoot
    color: "#2a2e31"
    property bool showFPS: false
    property bool deviceReady:false
    Text{
        id: deviceName
        color:"white"
        font.family: "Cambria"
        font.pixelSize: 20
        font.bold: true
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignBottom
        anchors{
            left:parent.left
            top:parent.top
            topMargin: 4
        }
        leftPadding: 16
        text: "Kraken Z3"
    }

    Rectangle{
        id: deviceSeparator
        height:1
        color:"white"
        anchors{
            left:parent.left
            leftMargin:12
            right:parent.right
            rightMargin:12
            top:deviceName.bottom
            topMargin: 2
        }
    }

    Text{
        id: liquidTempLabel
        color: "white"
        font.pixelSize: 20
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        anchors{
            left: parent.left
            top:deviceSeparator.bottom
            topMargin:2
            leftMargin:4
        }
        text:"Liquid Temperature: "
        font.family: "Cambria"
        font.letterSpacing: 4
        leftPadding: 8
    }
    Text{
        id: liquidTempValue
        color: "#5ce4f9"
        style: Text.Sunken
        styleColor:"#487071"
        font.pixelSize: 18
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        font.weight: Font.ExtraBold
        anchors{
            left: liquidTempLabel.right
            top:deviceSeparator.bottom
            right:parent.right
            topMargin:1
            leftMargin:4
        }
        text:KrakenZDriver.liquidTemperature.toString().slice(0,5) + " °C";
        font.family: "Cambria"
    }

    // Second Property, Pump Speed
    Text{
        id: pumpSpeedLabel
        color: "white"
        font.pixelSize: 20
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignBottom
        anchors{
            left: parent.left
            top:liquidTempLabel.bottom
            topMargin:4
            leftMargin:4
        }
        text:"Pump Speed: "
        font.family: "Cambria"
        font.letterSpacing: 4
        leftPadding: 8
    }
    Text{
        id: pumpSpeedValue
        color: "#97ffae"
        style: Text.Sunken
        styleColor:"#3d6d54"
        font.pixelSize:18
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        font.weight: Font.Bold
        anchors{
            left: pumpSpeedLabel.right
            top:pumpSpeedLabel.top
            topMargin:1
            leftMargin:4
        }
        text:KrakenZDriver.pumpSpeed + " RPM";
        font.family: "Cambria"
    }

    Text{
        id: fanSpeedLabel
        color: "white"
        font.pixelSize:20
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignBottom
        anchors{
            left: pumpSpeedLabel.left
            top:pumpSpeedLabel.bottom
            topMargin:2
        }
        text:"Fan Speed: "
        font.family: "Cambria"
        font.letterSpacing: 4
        leftPadding: 8
    }
    Text{
        id: fanSpeedValue
        color: "#97ffae"
        style: Text.Sunken
        styleColor:"#3d6d54"
        font.pixelSize:18
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        font.weight: Font.Bold
        anchors{
            left: fanSpeedLabel.right
            top:fanSpeedLabel.top
            right:parent.right
            topMargin:1
            leftMargin:4
        }
        text:KrakenZDriver.fanSpeed + " RPM";
        font.family: "Cambria"
    }

    Text{
        id: pumpDutyLabel
        color: "white"
        font.pixelSize: 20
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignBottom
        anchors{
            left: fanSpeedLabel.left
            top:fanSpeedLabel.bottom
            topMargin:4
        }
        text:"Pump Duty: "
        font.family: "Cambria"
        font.letterSpacing: 4
        leftPadding: 8
    }
    Text{
        id: pumpDutyValue
        color: "#fcffa6"
        style: Text.Sunken
        styleColor:"#887c42"
        font.pixelSize:18
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        font.weight: Font.Bold
        anchors{
            left: pumpDutyLabel.right
            top:pumpDutyLabel.top
            topMargin:1
            leftMargin:4
        }
        text:KrakenZDriver.pumpDuty + " %";
        font.family: "Cambria"
    }

    Text{
        id: fanDutyLabel
        color: "white"
        font.pixelSize:20
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignBottom
        anchors{
            left: pumpDutyLabel.left
            top:pumpDutyLabel.bottom
            topMargin:2
        }
        text:"Fan Duty: "
        font.family: "Cambria"
        font.letterSpacing: 4
        leftPadding: 8
    }
    Text{
        id: fanDutyValue
        color: "#fcffa6"
        style: Text.Sunken
        styleColor:"#887c42"
        font.pixelSize:18
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        font.weight: Font.Bold
        anchors{
            left: fanDutyLabel.right
            top:fanDutyLabel.top
            right:parent.right
            topMargin:1
            leftMargin:4
        }
        text:KrakenZDriver.fanDuty + " %";
        font.family: "Cambria"
    }
    LCDPreviewDelegate{
        id:krakenPreview
        anchors{
            top: pumpSpeedLabel.top
            right: parent.right
            rightMargin: 4
        }
        width:320
        height:320
        layer.enabled:true
        layer.effect:DropShadow{
            transparentBorder: true
            source:krakenPreview
            anchors.fill: krakenPreview
            horizontalOffset: 2
            verticalOffset: 10
            radius: 16
            spread:0
            samples:32
            color: "#000000"
        }
    }
    Item{
        id:imageOut
        anchors.centerIn: krakenPreview
        width:320
        height:320

        Rectangle{
            id: lens
            anchors.fill: parent
            color:"black"
            radius:width
            opacity:0
        }

        Text{
            function reset(){
                errorTitle.visible = false;
                errorText.text = "";
            }
            id:errorTitle
            anchors.top:parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 16
            text:"QML Error"
            color:"lightblue"
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 24
            visible:false
        }

        Text{
            id:errorText
            anchors.centerIn:parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color:"white"
            width:280
            wrapMode:Text.WrapAnywhere
            visible:errorTitle.visible
        }
        Rectangle{
            visible: krakenRoot.showFPS && (AppController.mode > AppMode.STATIC_IMAGE)
            anchors.bottom:parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            height: 28
            color: "#120084"
            width: 110
            radius:6
            gradient: Gradient {
                GradientStop {
                    position: 0.00;
                    color: "#0000ff";
                }
                GradientStop {
                    position: 0.95;
                    color: "#161949";
                }
            }
            Text{
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 18
                style: Text.Sunken
                styleColor: "#01767a"
                color:"white"
                text: "FPS: " + KrakenZDriver.fps.toString().slice(0,5)
                font.family: "Comic Sans MS"
            }
        }
    }
    Rectangle{
        id:builtinMode
        visible: AppController.mode == AppMode.BUILT_IN
        anchors.centerIn: krakenPreview
        width:320
        height:320
        color: "#22262b"
        radius:160
        border.width: 2
        border.color: "#5c5c5c"
        Text{
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 24
            font.family: "Comic Sans MS"
            text:"OEM\nDisplay Mode"
            style:Text.Sunken
            color:"white"
        }
    }

    Rectangle{
        width:48
        height:width
        radius:width
        visible:AppController.mode === AppMode.GIF_MODE
        anchors{
            bottom:krakenPreview.bottom
            right:krakenPreview.right
            rightMargin: 8
        }
        Image{
            id: ppIcon
            height: 32
            width:32
            anchors.centerIn: parent
            source: AppController.animationPlaying ?  "qrc:/images/pause.svg" : "qrc:/images/play.svg"
            antialiasing: true
            smooth: true
        }

        color:"white"
        MouseArea{
            anchors.fill: parent
            onClicked: {
                AppController.animationPlaying = !AppController.animationPlaying
            }
        }
    }

    Rectangle{
        id: qmlOptions
        visible: AppController.mode >= AppMode.GIF_MODE
        color: "#8d8d8d"
        anchors.top:krakenPreview.bottom
        anchors.topMargin: 8
        radius:6
        border.color: "#004d4d4d"
        height:AppController.mode > AppMode.GIF_MODE ?  206:138
        anchors.left: krakenPreview.left
        anchors.right: krakenPreview.right
        Text{
            id:qmlTitle
            text:"Qml Options"
            font.bold: true
            font.pixelSize: 20
            font.family: "Cambria"
            horizontalAlignment: Text.AlignHCenter
            anchors{
                left:parent.left
                right:parent.right
                top:parent.top
            }
        }

        Column{
            width:parent.width -4
            anchors{
                top:qmlTitle.bottom
                bottom:parent.bottom
                topMargin: 2
                horizontalCenter: parent.horizontalCenter

            }
            spacing: 4
            Rectangle{
                border.color: "black"
                radius:4
                height:32
                width:parent.width - 8
                color:"orange"
                anchors.horizontalCenter: parent.horizontalCenter
                Text{
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text:"Reload"
                    color:"white"
                }
                MouseArea{
                    anchors.fill: parent
                    onClicked:{
                        AppController.loadQmlFile(AppController.loadedPath);
                    }
                }
            }
            Rectangle{
                border.color: "black"
                radius:4
                height:32
                width:parent.width - 8
                color:krakenRoot.showFPS ? "red" : "#cc03d429"
                anchors.horizontalCenter: parent.horizontalCenter
                Text{
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text:krakenRoot.showFPS ? "Hide FPS" : "Show FPS";
                    color:"white"
                }
                MouseArea{
                    anchors.fill: parent
                    onClicked:{
                        krakenRoot.showFPS = !krakenRoot.showFPS;
                        if(!krakenRoot.showFPS) {
                            AppController.drawFPS = false;
                        } else {
                            AppController.drawFPS = drawCheck.checked;
                        }
                    }
                }
            }
            Rectangle{
                border.color: "black"
                radius:4
                visible: krakenRoot.showFPS
                height:32
                width:parent.width - 8
                color:AppController.drawFPS ? "red" : "#cc03d429"
                anchors.horizontalCenter: parent.horizontalCenter
                CheckBox{
                    id: drawCheck
                    anchors.centerIn: parent
                    anchors.horizontalCenterOffset: -48
                    indicator.width: 24
                    indicator.height: 24
                    checked: AppController.drawFPS
                    contentItem:Text{
                        leftPadding: 116
                        text: "Draw FPS on LCD";
                        color:"white"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                    spacing:4
                    onCheckedChanged: {
                        AppController.drawFPS = checked;
                    }
                }
            }
            Rectangle{
                height:48
                width:parent.width-8
                color:"transparent"
                visible: AppController.mode > AppMode.GIF_MODE
                Text{
                    id: lowDelay
                    color:"white"
                    text: setDelay.from
                    leftPadding:6
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    anchors{
                        left: parent.left
                        verticalCenter: setDelay.verticalCenter
                    }
                }

                Slider{
                    id:setDelay
                    anchors{
                        top:parent.top
                        left:lowDelay.right
                        right:highDelay.left
                    }
                    snapMode:Slider.SnapAlways
                    live:false
                    from: 40
                    to: 1000
                    stepSize:5
                    value:AppController.frameDelay
                    handle:Rectangle{
                        color: "#655e71"
                        border.color: "#b9b9b9"
                        border.width: 2
                        height: parent.height * .8
                        width: height
                        radius:width
                        x: setDelay.leftPadding + setDelay.visualPosition * (setDelay.availableWidth - width)
                        y: setDelay.topPadding + setDelay.availableHeight / 2 - height / 2
                    }
                    background: Rectangle {
                        x: setDelay.leftPadding
                        y: setDelay.topPadding + setDelay.availableHeight / 2 - height / 2
                        implicitWidth: 200
                        implicitHeight: 16
                        width: setDelay.availableWidth
                        height: setDelay.height *.35
                        radius: 2
                        color: "#e6e6e6"
                        border.width: 2
                        border.color:"#7e7e7e"

                        Rectangle {
                            width: setDelay.visualPosition * parent.width
                            height: parent.height
                            color: "#cc03d429"
                            radius: 2
                        }
                    }

                    onValueChanged: {
                        delayValue.text  = "frame delay " + setDelay.value + "ms"
                        AppController.frameDelay = value;
                    }

                    height: 36
                }
                Text{
                    id: delayValue
                    color:"black"
                    font.pixelSize: 16
                    font.bold: true
                    anchors{
                        horizontalCenter: setDelay.horizontalCenter
                        top:setDelay.bottom
                        topMargin: -8
                    }
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                Text{
                    id: highDelay
                    color:"white"
                    text:"1000"
                    rightPadding:4
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    anchors{
                        verticalCenter: setDelay.verticalCenter
                        right:parent.right
                    }
                }
            }
        }
    }

    Text{
        id: setFanLabel
        text:"Set Fan Duty"
        font.family: "Constantia"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        anchors{
            top: fanDutyValue.bottom
            topMargin:18
            left: parent.left
        }
        font.pixelSize: 22
        color:"white"
        style: Text.Sunken
        styleColor: "#737272"
        font.letterSpacing: 4
        leftPadding: 8

    }

    Text{
        id: zeroFanLabel
        color:"white"
        text:"0"
        leftPadding:16
        horizontalAlignment: Text.AlignHCenter
        anchors{
            left: parent.left
            verticalCenter: setFanSlider.verticalCenter
        }
    }

    Slider{
        id:setFanSlider
        anchors{
            top:setFanLabel.bottom
            topMargin:2
            left:zeroFanLabel.right
            right:fullFanLabel.left
        }
        snapMode:Slider.SnapAlways
        live:false
        from: 0
        to: 100
        stepSize:1
        value:KrakenZDriver.fanDuty
        handle:Rectangle{
            color: "#655e71"
            border.color: "#b9b9b9"
            border.width: 2
            height: parent.height * .8
            width: height
            radius:width
            x: setFanSlider.leftPadding + setFanSlider.visualPosition * (setFanSlider.availableWidth - width)
            y: setFanSlider.topPadding + setFanSlider.availableHeight / 2 - height / 2
        }
        background: Rectangle {
            x: setFanSlider.leftPadding
            y: setFanSlider.topPadding + setFanSlider.availableHeight / 2 - height / 2
            implicitWidth: 200
            implicitHeight: 16
            width: setFanSlider.availableWidth
            height: setFanSlider.height *.35
            radius: 2
            color: "#e6e6e6"
            border.width: 2
            border.color:"#7e7e7e"

            Rectangle {
                width: setFanSlider.visualPosition * parent.width
                height: parent.height
                color: "#cc03d429"
                radius: 2
            }
        }

        onValueChanged: {
            if(krakenRoot.deviceReady) {
                KrakenZDriver.setFanDuty(value);
            }
        }

        height: 36
    }
    Text{
        id: fanValue
        color:"white"
        text:setFanSlider.value + "%"
        font.pixelSize: 16
        font.bold: true
        anchors{
            horizontalCenter: setFanSlider.horizontalCenter
            top:setFanSlider.bottom
            topMargin: 2
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Text{
        id: fullFanLabel
        color:"white"
        text:"100"
        rightPadding:4
        horizontalAlignment: Text.AlignHCenter
        anchors{
            verticalCenter: setFanSlider.verticalCenter
            right:krakenPreview.left
            rightMargin: 24
        }
    }

    Text{
        id: setPumpLabel
        text:"Set Pump Duty"
        font.family: "Constantia"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        anchors{
            top: fanValue.bottom
            left:parent.left
            topMargin:2
        }
        font.pixelSize: 22
        color:"white"
        style: Text.Sunken
        styleColor: "#737272"
        font.letterSpacing: 4
        leftPadding: 8
    }

    Text{
        id: zeroPumpLabel
        color:"white"
        text:"20"
        leftPadding:16
        horizontalAlignment: Text.AlignHCenter
        anchors{
            left: parent.left
            verticalCenter: setPumpSlider.verticalCenter
        }
    }

    Slider{
        id:setPumpSlider
        anchors{
            top:setPumpLabel.bottom
            topMargin:2
            left:zeroPumpLabel.right
            right:fullPumpLabel.left
        }
        snapMode:Slider.SnapAlways
        live:false
        from: 20
        to: 100
        stepSize:1
        handle:Rectangle{
            color: "#655e71"
            border.color: "#b9b9b9"
            border.width: 2
            height: parent.height * .8
            width: height
            radius:width
            x: setPumpSlider.leftPadding + setPumpSlider.visualPosition * (setPumpSlider.availableWidth - width)
            y: setPumpSlider.topPadding + setPumpSlider.availableHeight / 2 - height / 2
        }
        background: Rectangle {
            x: setPumpSlider.leftPadding
            y: setPumpSlider.topPadding + setPumpSlider.availableHeight / 2 - height / 2
            implicitWidth: 200
            implicitHeight: 16
            width: setPumpSlider.availableWidth
            height: setPumpSlider.height *.35
            radius: 2
            color: "#e6e6e6"
            border.width: 2
            border.color:"#7e7e7e"

            Rectangle {
                width: setPumpSlider.visualPosition * parent.width
                height: parent.height
                color: "#cc03d429"
                radius: 2
            }
        }
        height: 36
        onValueChanged: {
            if(krakenRoot.deviceReady){
                KrakenZDriver.setPumpDuty(value);
            }
        }

    }
    Text{
        id: pumpValue
        color:"white"
        text:setPumpSlider.value + "%"
        font.pixelSize: 16
        font.bold: true
        anchors{
            horizontalCenter: setPumpSlider.horizontalCenter
            top:setPumpSlider.bottom
            topMargin: 2
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Text{
        id: fullPumpLabel
        color:"white"
        text:"100"
        rightPadding:4
        horizontalAlignment: Text.AlignHCenter
        anchors{
            verticalCenter: setPumpSlider.verticalCenter
            right:fullFanLabel.right
        }
    }
    Text{
        id: setBrightnessLabel
        text:"Set LCD Brightness"
        font.family: "Constantia"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        anchors{
            top: pumpValue.bottom
            left:parent.left
            topMargin:2
        }
        font.pixelSize: 22
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
        value:KrakenZDriver.brightness
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
            if(value > 50){
                lens.color = "white";
                lens.opacity = value/100 - 0.85;
            } else {
                lens.color = "black";
                if(value == 0){
                    lens.opacity = 1.0;
                }else {
                    lens.opacity = (50 - value)/100;
                }
            }
            if(krakenRoot.deviceReady) {
                KrakenZDriver.setBrightness(value);
            }
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
            right:fullFanLabel.right
        }
    }

    Text{
        id: setOrientationLabel
        text:"Set Orientation"
        font.family: "Constantia"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        anchors{
            top: brightnessValue.bottom
            left:parent.left
            topMargin:2
        }
        font.pixelSize: 22
        color:"white"
        style: Text.Sunken
        styleColor: "#737272"
        font.letterSpacing: 4
        leftPadding: 8
    }

    Text{
        id: leftOrientationLabel
        color:"white"
        text:"0"
        leftPadding:16
        horizontalAlignment: Text.AlignHCenter
        anchors{
            left: parent.left
            verticalCenter: setOrientationSlider.verticalCenter
        }
    }

    Slider{
        id:setOrientationSlider
        anchors{
            top:setOrientationLabel.bottom
            topMargin:2
            left:leftOrientationLabel.right
            right:rightOrientationLabel.left
        }
        snapMode:Slider.SnapOnRelease
        live:true
        from: 0
        to: 270
        stepSize:unlockRotation.checked ? 90:1
        handle:Rectangle{
            color: "#655e71"
            border.color: "#b9b9b9"
            border.width: 2
            height: parent.height * .8
            width: height
            radius:width
            x: setOrientationSlider.leftPadding + setOrientationSlider.visualPosition * (setOrientationSlider.availableWidth - width)
            y: setOrientationSlider.topPadding + setOrientationSlider.availableHeight / 2 - height / 2
        }
        background: Rectangle {
            x: setOrientationSlider.leftPadding
            y: setOrientationSlider.topPadding + setOrientationSlider.availableHeight / 2 - height / 2
            implicitWidth: 200
            implicitHeight: 16
            width: setOrientationSlider.availableWidth
            height: setOrientationSlider.height *.35
            radius: 2
            color: "#e6e6e6"
            border.width: 2
            border.color:"#7e7e7e"

            Rectangle {
                width: setOrientationSlider.visualPosition * parent.width
                height: parent.height
                color: "#cc03d429"
                radius: 2
            }
        }

        onValueChanged: {
            if(krakenRoot.deviceReady) {
                AppController.setOrientationFromAngle(value);
            }
        }

        height: 36
    }

    CheckBox{
        id: unlockRotation
        width:24
        height:24
        anchors{
            verticalCenter: orientationValue.verticalCenter
            left:parent.left
            leftMargin:18
        }
    }
    Text{
        anchors{
            left:unlockRotation.right
            leftMargin:8
            verticalCenter:unlockRotation.verticalCenter
        }
        text:unlockRotation.checked ? "Unlock step size":"Lock step to 90"
        color:"white"
        font.pixelSize: 16
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    Text{
        id: orientationValue
        color:"white"
        text:setOrientationSlider.value + "°"
        font.pixelSize: 16
        font.bold: true
        anchors{
            right: setOrientationSlider.right
            top:setOrientationSlider.bottom
            topMargin: 2
        }
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
    }

    Text{
        id: rightOrientationLabel
        color:"white"
        text:"270"
        rightPadding:16
        horizontalAlignment: Text.AlignHCenter
        anchors{
            verticalCenter: setOrientationSlider.verticalCenter
            right:fullFanLabel.right
        }
    }

    // ACTIONS
    Text{
        id: actionsText
        color:"white"
        font.family: "Cambria"
        font.pixelSize: 22
        font.bold: true
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignBottom
        anchors{
            left:parent.left
            bottom:actionsSeparator.bottom
            bottomMargin: 6
        }
        leftPadding: 16
        text: "Actions"
    }
    Rectangle{
        id: actionsSeparator
        height:1
        color:"white"
        anchors{
            left:parent.left
            leftMargin:12
            right:parent.right
            rightMargin:12
            bottom: actionView.top
            bottomMargin: 4
        }
    }
    ObjectModel{
        id: actionModel
        Rectangle{
            height:48
            color: "#b7b7b7"
            width:74
            Text{
                anchors.fill: parent
                text:"Set Image"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    if(fileLoader.folder.length == 0){
                        fileLoader.folder = ApplicationData;
                    }
                    fileLoader.filterIndex = 0;
                    fileLoader.active = true;
                }
            }
        }
        Rectangle{
            height:48
            color: "#b7b7b7"
            width:74
            Text{
                anchors.fill: parent
                text:"Monitor Mode"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    AppController.setBuiltIn(false);
                    KrakenZDriver.setNZXTMonitor();
                }
            }
        }
        Rectangle{
            height:48
            color: "#b7b7b7"
            width:74
            Text{
                anchors.fill: parent
                text:"Set Qml"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    if(fileLoader.folder.length == 0){
                        fileLoader.folder = ApplicationData;
                    }
                    fileLoader.filterIndex = 1;
                    fileLoader.active = true;
                }
            }
        }
        Rectangle{
            height:48
            color: "#b7b7b7"
            width:74
            Text{
                anchors.fill: parent
                text:"Set Loading GIF"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }            
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    AppController.setBuiltIn(true);
                    KrakenZDriver.setBuiltIn(1);
                }
            }
        }
        Rectangle{
            height:48
            color: "#b7b7b7"
            width:74
            Text{
                anchors.fill: parent
                text:"Turn LCD Off"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }

            MouseArea{
                anchors.fill: parent
                onClicked: {
                    setBrightnessSlider.value = 0;
                }
            }
        }
    }

    Loader{
        id: fileLoader
        active: false
        property int filterIndex : 0
        property string folder:""
        sourceComponent: FileDialog{
            nameFilters:["Image Files (*.jpg *.jpeg *.png *.gif *.tif *.svg)", "Qml Application (*.qml)", "Something Else?(*.*)"]
            selectedNameFilter.index: fileLoader.filterIndex
            onAccepted: {
                if(files.length > 0){
                    var selectedPath = files[0].toString();
                    fileLoader.folder = folder;
                    if(selectedPath.indexOf(".qml") >= 0){ // app
                        console.log("Loading Qml: " + selectedPath);
                        AppController.loadQmlFile(selectedPath)
                    }else{
                        console.log("Setting image: " + selectedPath);
                        AppController.loadImage(selectedPath);
                    }
                }
                fileLoader.active = false;
            }
            onRejected: {
                fileLoader.active = false;
            }
        }
        onStatusChanged: {
            if(status == Loader.Ready) {
                item.folder = AppController.getLocalFolderPath(fileLoader.folder);
                item.open(item.folder)
            }
        }
    }

    ListView{
        id: actionView
        anchors{
            left:parent.left
            right:parent.right
            margins:4
            leftMargin:24
            bottom:parent.bottom
            bottomMargin:12
        }
        height: 72
        spacing: 42
        orientation: ListView.Horizontal
        model: actionModel
    }
    Connections{
        target: AppController
        function onAppReady(){
            errorTitle.reset();
        }
        function onModeChanged(mode) {
            errorTitle.reset();
        }

        function onQmlFailed(error) {
            errorText.text = error;
            errorTitle.visible = true;
        }
    }
    Connections{
        target: KrakenZDriver
        function onDeviceReady(){
            krakenRoot.deviceReady = true;
            setPumpSlider.value = Qt.binding(function(){return KrakenZDriver.pumpDuty});
            setFanSlider.value = Qt.binding(function(){return KrakenZDriver.fanDuty});
            setBrightnessSlider.value = Qt.binding(function(){return KrakenZDriver.brightness});
            setOrientationSlider.value = Qt.binding(function(){return KrakenZDriver.rotationOffset});
        }

    }

    Component.onCompleted:{
        SystemTray.preventCloseAppWithWindow();
        SettingsManager.applyStartupProfile();
        unlockRotation.checked = (KrakenZDriver.rotationOffset % 90 == 0)
    }
}
