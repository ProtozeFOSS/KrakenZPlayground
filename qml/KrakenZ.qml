import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import com.krakenzplayground.app 1.0;

Rectangle {
    id: krakenRoot
    color: "#2a2e31"
    property bool advanced: false
    property bool drawFPS: false
    property int  frameDelayMS:10
    property int  mode:0 // 0 for nothing, 1 for image, 2 for animated image (gif), 3 for qml
    property string selectedPath:""
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
        text: "Kraken Z73"
    }

    Rectangle{
        id: advancedButton
        anchors{
            left: deviceName.right
            leftMargin: 480 - deviceName.paintedWidth
            verticalCenter: deviceName.verticalCenter
        }
        width: 88
        radius:4
        Text{
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color:"lightgrey"
            text:"Advanced"
            font.bold: true
        }

        MouseArea{
            anchors.fill: parent
            onClicked:{
                krakenRoot.advanced = !krakenRoot.advanced;
            }
        }

        height:deviceName.paintedHeight - 4
        color: "#5a5a5a"
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
        font.pixelSize: 28
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
        font.pixelSize: 34
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
        font.pixelSize: 24
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
        font.pixelSize:24
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
        font.pixelSize:24
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
        font.pixelSize:24
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
        font.pixelSize: 24
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
        font.pixelSize:24
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
        font.pixelSize:24
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
        font.pixelSize:24
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
        anchors.top: pumpSpeedLabel.top
        anchors.left: pumpSpeedValue.right
        anchors.leftMargin: -42
        width:320
        height:320
        property int frame:0
        animationImage.onFrameChanged: {
            krakenPreview.grabToImage(
               function(result) {
                   krakenPreview.frame = !krakenPreview.frame
                   KrakenZDriver.setImage(result.image, krakenPreview.frame, true);
               });
        }

        animationImage.onPlayingChanged: {
            if( animationImage.playing){
                KrakenZDriver.startMonitoringFramerate();
            }else {
                KrakenZDriver.stopMonitoringFramerate();
                krakenPreview.grabToImage(function(result) {
                                           krakenPreview.frame = !krakenPreview.frame
                                           KrakenZDriver.setImage(result.image, krakenPreview.frame, true);
                                       });
            }
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
        Rectangle{
            visible: KrakenZDriver.fps !== 0
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
        width:48
        height:width
        radius:width
        visible:krakenPreview.animated
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
            source: "qrc:/images/pause.svg"
            antialiasing: true
            smooth: true
        }

        color:"white"
        MouseArea{
            anchors.fill: parent
            onClicked: {
                krakenPreview.animationImage.playing = !krakenPreview.animationImage.playing
                if(krakenPreview.animationImage.playing) {
                    ppIcon.source = "qrc:/images/pause.svg"
                } else {
                    ppIcon.source = "qrc:/images/play.svg"
                }
            }
        }
    }

    Rectangle{
        id: qmlOptions
        color: "#8d8d8d"
        anchors.top:krakenPreview.bottom
        anchors.topMargin: 8
        radius:6
        border.color: "#004d4d4d"
        height:180
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
                horizontalCenter: parent.horizontalCenter

            }
            spacing: 4
            Rectangle{
                border.color: "black"
                radius:4
                height:32
                width:parent.width
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
                        userAppController.loadQmlFile(krakenRoot.selectedPath);
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
            topMargin:2
            left: parent.left
        }
        font.pixelSize: 24
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
            if(KrakenZDriver){
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
        font.pixelSize: 24
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
        value:KrakenZDriver.pumpDuty
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

        onValueChanged: {
            KrakenZDriver.setPumpDuty(value);
        }

        height: 36
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

            KrakenZDriver.setBrightness(value);
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
        font.pixelSize: 24
        color:"white"
        style: Text.Sunken
        styleColor: "#737272"
        font.letterSpacing: 4
        leftPadding: 8
    }

    Text{
        id: leftOrientationLabel
        color:"white"
        text:"-180"
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
        snapMode:Slider.SnapAlways
        live:true
        from: -180
        to: 180
        stepSize:90
        value: KrakenZDriver.rotationOffset
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
            KrakenZDriver.setRotationOffset(value);
        }

        height: 36
    }
    Text{
        id: orientationValue
        color:"white"
        text:setOrientationSlider.value + "%"
        font.pixelSize: 16
        font.bold: true
        anchors{
            horizontalCenter: setOrientationSlider.horizontalCenter
            top:setOrientationSlider.bottom
            topMargin: 2
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Text{
        id: rightOrientationLabel
        color:"white"
        text:"180"
        rightPadding:16
        horizontalAlignment: Text.AlignHCenter
        anchors{
            verticalCenter: setOrientationSlider.verticalCenter
            right:fullFanLabel.right
        }
    }

    Text{
        id: detailsText
        color:"white"
        font.family: "Cambria"
        font.pixelSize: 20
        font.bold: true
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignBottom
        anchors{
            left:parent.left
            top:krakenPreview.bottom
            topMargin: 180
        }
        leftPadding: 12
        text: "Details"
    }
    Rectangle{
        id: statusSeparator
        height:1
        color:"white"
        anchors{
            left:parent.left
            leftMargin:12
            right:parent.right
            rightMargin:12
            top:detailsText.bottom
            topMargin: 2
        }
    }


    Text{
        id: actionsText
        color:"white"
        font.family: "Cambria"
        font.pixelSize: 20
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
                    if(KrakenZDriver.content){
                        KrakenZDriver.clearContentItem();
                    }
                    krakenPreview.animated = false;
                    krakenPreview.animationImage.playing = false;
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
                    if(KrakenZDriver.content){
                        KrakenZDriver.clearContentItem();
                    }
                    krakenPreview.animated = false;
                    krakenPreview.animationImage.playing = false;
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
                    KrakenZDriver.setBrightness(0);
                }
            }
        }
    }

    Loader{
        id: fileLoader
        active: false
        sourceComponent: FileDialog{
            onAccepted: {
                if(fileUrls.length > 0){
                    console.log(fileUrls);
                    krakenRoot.selectedPath = fileUrls[0].toString().replace("file://","");
                    if(krakenRoot.selectedPath.indexOf(".qml") >= 0){
                        console.log("Loading Qml File: " + krakenRoot.selectedPath);
                        userAppController.loadQmlFile(krakenRoot.selectedPath)
                        krakenPreview.animated = false;
                        krakenPreview.animationImage.playing = false;
                        KrakenZDriver.setContent(userApp,krakenRoot.frameDelayMS);
                    }else{
                        console.log("Setting image URL: " + fileUrls[0].toString());
                        console.log("Setting image file path: " + krakenRoot.selectedPath);
                        if(KrakenZDriver.content){
                            KrakenZDriver.clearContentItem();
                        }

                        if(fileUrls[0].toString().indexOf(".gif") >= 0){
                            krakenPreview.animationImage.source = fileUrls[0].toString();
                            krakenPreview.animated = true;
                            krakenPreview.animationImage.playing = true;
                        }else {
                            KrakenZDriver.setImage(krakenRoot.selectedPath);
                            krakenPreview.animated = false;
                            krakenPreview.animationImage.playing = false;
                        }
                    }
                }
                fileLoader.active = false;
            }
            onRejected: {
                fileLoader.active = false;
            }
        }
        onItemChanged: {
            if(item){
                item.open()
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
        }
        height: 72
        spacing: 42
        orientation: ListView.Horizontal
        model: actionModel
    }
    // Advanced Section
    KrakenAdvanced{
        id:krakenAdvanced
        visible:true
        anchors.left:parent.left
        anchors.leftMargin: 602
        anchors.top:parent.top
        anchors.topMargin:16
        anchors.bottom: parent.bottom
        onStopAnimation: {
            krakenPreview.animated = false;
            krakenPreview.animationImage.playing = false;
        }
    }
    Timer{
        id:statusPoll
        interval:500
        repeat: true
        running: true
        onTriggered: {
            KrakenZDriver.sendStatusRequest();
        }
    }
    KrakenAppController{
        id:userAppController
        container:userAppContainer

    }

    Item{
        id:userApp
        height:320
        width:320
        y:800
        Item{
            id:userAppContainer
            anchors.fill: parent

        }

        Rectangle{
            visible:krakenRoot.drawFPS
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
}
