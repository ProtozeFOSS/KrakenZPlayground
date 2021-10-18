import QtQuick 2.15
import QtQml.Models 2.15
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
Rectangle {
    id: krakenRoot
    color: "#2a2e31"
    property bool advanced: false

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
            topMargin: 6
        }
        leftPadding: 16
        text: "Kraken Z73"
    }
    Connections{
        target:KrakenZDriver
        function onFanDutyChanged(duty){
            setFanSlider.value = duty;
        }
        function onPumpDutyChanged(duty){
            setPumpSlider.value = duty;

        }

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
            topMargin: 4
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
            topMargin:4
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
            topMargin:3
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
            topMargin:6
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
            topMargin:3
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
            topMargin:4
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
            topMargin:3
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
            topMargin:6
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
            topMargin:3
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
            topMargin:4
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
            topMargin:3
            leftMargin:4
        }
        text:KrakenZDriver.fanDuty + " %";
        font.family: "Cambria"
    }
    LCDPreviewDelegate{
        id:krakenPreview
        anchors.top: pumpSpeedLabel.top
        anchors.left: pumpSpeedValue.right
        anchors.leftMargin: -52
        width:320
        height:320
        property int frame:0
        animationImage.onFrameChanged: {
            krakenPreview.grabToImage(function(result) {
                                       KrakenZDriver.moveToBucket(krakenPreview.frame);
                                       krakenPreview.frame = !krakenPreview.frame
                                       KrakenZDriver.setImage(result.image, krakenPreview.frame, false);
                                   });
        }

        onAnimatedChanged: {
            if(animated){
                KrakenZDriver.setContent(krakenPreview,1);
            }else {
                KrakenZDriver.clearContentItem();
            }
        }
    }
    Item{
        id:imageOut
        anchors.centerIn: krakenPreview
        width:320
        height:320
        Rectangle{
            anchors.bottom:parent.bottom
            anchors.bottomMargin: 2
            anchors.horizontalCenter: parent.horizontalCenter
            height: 24
            width: 140
            color: "red"
            Text{
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: 20
                color:"white"
                text: "FPS: " + KrakenZDriver.fps.toString().slice(0,5)
            }
        }
        MouseArea{
            anchors.fill: parent
            onClicked:{
                KrakenZDriver.setContent(imageOut,100);
            }
        }
    }

    Rectangle{
        width:48
        height:width
        radius:width
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

    Text{
        id: setFanLabel
        text:"Set Fan Duty"
        font.family: "Constantia"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        anchors{
            top: fanDutyValue.bottom
            topMargin:4
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
        text:"0%"
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
                console.log("Setting Fan Duty to " + value);
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
        text:"100%"
        rightPadding:16
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
            topMargin:4
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
        text:"0%"
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
        from: 0
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

        onValueChanged: {
            console.log("Set Fan Duty: " + value + "%");
            //KrakenZDriver.setPumpDuty(value);
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
        text:"100%"
        rightPadding:16
        horizontalAlignment: Text.AlignHCenter
        anchors{
            verticalCenter: setPumpSlider.verticalCenter
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
            topMargin: -8
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
            top:statusSeparator.bottom
            topMargin: 6
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
            top:actionsText.bottom
            topMargin: 4
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
                text:"Clear\n(Blackout)"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }

            MouseArea{
                anchors.fill: parent
                onClicked: {
                    KrakenZDriver.setBrightness(0);
                    KrakenZDriver.blankScreen();
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
                    console.log("Setting image: " + fileUrls[0]);
                    if(fileUrls[0].toString().indexOf(".gif") >= 0){
                        krakenPreview.animationImage.source = fileUrls[0].toString();
                        krakenPreview.animated = true;
                        krakenPreview.animationImage.playing = true;
                    }else {
                        KrakenZDriver.setImage(fileUrls[0].toString());
                        krakenPreview.animated = false;
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
        anchors{
            top:actionsSeparator.bottom
            left:parent.left
            right:parent.right
            margins:4
            leftMargin:120
        }

        spacing: 24


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

    Component.onCompleted: {
        KrakenZDriver.rotationOffset = -90;
        //animationTimer.start();
    }
}
