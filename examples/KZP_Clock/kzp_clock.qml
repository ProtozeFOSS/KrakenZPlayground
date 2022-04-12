import QtQuick 2.15
import QtGraphicalEffects 1.15
Rectangle{
    id:top
    anchors.fill:parent
    property int hoursPassed:0
    color:"white"
    property var  weekDays : []
    property var  months : []
    function setArrays(locale)
    {
        const options = { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' };
        var baseDate = new Date(Date.UTC(2017, 0, 2));
        for(var i = 0; i < 7; i++)
        {
            top.weekDays.push(baseDate.toDateString(locale).slice(0,3));
            baseDate.setDate(baseDate.getDate() + 1);
        }
        for(var j = 0; j < 12; ++j) {
            baseDate.setMonth(j);
            top.months.push(baseDate.toDateString(locale).slice(4,7));
        }
    }

    function setCompassText(offset : int) {
        if(offset < 0) {
            offset += 360;
        }
        if(offset >= 0 && offset < 90) { // north
            compassText.text = "N";
            compassText.color = "#27cbfd"
            return;
        }
        if(offset >= 90 && offset < 180) { // north
            compassText.text = "E";
            compassText.color = "#db8c0d"
            return;
        }
        if(offset >= 180 && offset < 270) { // north
            compassText.text = "S";
            compassText.color = "#00e64d"
            return;
        }
        if(offset >= 270 && offset < 360) { // north
            compassText.text = "W";
            compassText.color = "#e34aee"
            return;
        }

    }
    Connections {
        target: KrakenZDriver
        function onRotationOffsetChanged(offset : int){top.setCompassText(offset);}
    }

    property real lastHour: 0
    property bool is24Hour: false
    property bool isAm: true
    property string dateText: ""
    Connections {
        target: AppController
        function onDraw() {
            const options = { weekday: 'long', day: 'numeric' };
            var date = new Date();
            var hours = date.getHours() + top.hoursPassed;
            if(top.lastHour != hours) {
                top.lastHour = hours;
                globe.currentFrame = (80 + (hours * 6)) % 180; // update the globe
                if(top.is24Hour) {  // set values for 24 hour clock
                    top.currentHour = hours;
                    top.isAm = false;
                } else {
                    if(hours >= 12) {
                        top.isAm = false;
                        if(hours > 12){
                            hours -= 12;
                        }
                    }else {
                        top.isAm = true;
                        if(hours == 0) {
                            hours = 12;
                        }
                    }
                }
                hourText.text = hours.toString().padStart(2, '0')
                // an hour went by? maybe it is a new day!
                top.dateText = top.weekDays[date.getDay()] + '  ' + date.getDate().toString().padStart(2,'0');
                monthText.text = top.months[date.getMonth()]
                yearText.text = date.getFullYear()
            }
            minuteText.text = date.getMinutes().toString().padStart(2, '0')
            secondsText.text = date.getSeconds().toString().padStart(2, '0')
            fanRPM.text = KrakenZDriver.fanSpeed
            pumpDuty.text = KrakenZDriver.pumpDuty
        }
    }

    FontLoader {
        id: clockFont
        source:"next-art.heavy.otf"
    }
    FontLoader {
        id: numberFont
        source:"next-art.heavy.otf"
    }

    Image{
        anchors{
            verticalCenter:parent.verticalCenter
            horizontalCenter: parent.horizontalCenter
            horizontalCenterOffset: 2
        }
        fillMode: Image.PreserveAspectFit
        source:"background.png"
        height:324
        width:328
        smooth:true
    }
    // Time Text
    Text{
        id: hourText
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        anchors{
            horizontalCenter: parent.horizontalCenter
            verticalCenter:parent.verticalCenter
            verticalCenterOffset: -60
            horizontalCenterOffset:36
        }
        color:"#ffffff"
        font.family: numberFont.name
        font.letterSpacing: 2
        font.pixelSize: 92
        font.bold: true
        style:Text.Outline
        styleColor: "#000000"
        layer.enabled: true
        layer.effect: InnerShadow{
            radius:6
            spread:.75
            samples:32
            color:"black"
            cached: false
          }
    }
    Text{
        id: monthText
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        anchors{
            horizontalCenter: dateText.horizontalCenter
            verticalCenter: parent.verticalCenter
            verticalCenterOffset:-90
        }
        color:"white"
        font.family: clockFont.name
        font.pixelSize: 22
        font.letterSpacing: -1
        font.bold: true
        style:Text.Sunken
        styleColor:"black"
        layer.enabled: true
        layer.effect: InnerShadow{
            opacity:1
            spread:.7
            samples:8
            color:"black"
            cached: false
        }
    }

    Text{
        id: dateText
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        anchors{
            verticalCenter: parent.verticalCenter
            left:parent.left
            verticalCenterOffset:-52
            leftMargin:24
        }
        color:"white"
        font.family: clockFont.name
        font.pixelSize: 22
        font.letterSpacing: -1
        font.bold: true
        text: top.dateText
        style:Text.Sunken
        styleColor:"black"
        layer.enabled: true
        layer.effect: InnerShadow{
            opacity:1
            spread:.7
            samples:8
            color:"black"
            cached: false
        }
    }
    Text{
        id: yearText
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        anchors{
            horizontalCenter: dateText.horizontalCenter
            verticalCenter: parent.verticalCenter
            verticalCenterOffset:-31
        }
        color:"white"
        font.family: clockFont.name
        font.pixelSize: 14
        font.letterSpacing: -1
        font.bold: true
        text:""
        style:Text.Sunken
        styleColor:"black"
        layer.enabled: true
        layer.effect: InnerShadow{
            opacity:1
            spread:.7
            samples:8
            color:"black"
            cached: false
        }
    }



    Text{
        id: secondsText
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        anchors{
            horizontalCenter: parent.horizontalCenter
            verticalCenter:parent.verticalCenter
            horizontalCenterOffset:126
        }
        color:"#fdaa1c"
        font.family: numberFont.name
        font.pixelSize: 34
        style:Text.Outline
        styleColor: "#000000"
        layer.enabled: true
        layer.effect: InnerShadow{
            radius:2
            spread:.75
            samples:32
            color:"black"
            cached: false
          }
    }

    // Time Text
    Text{
        id: fanRPM
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors{
            left:parent.left
            leftMargin:26
            verticalCenter:parent.verticalCenter
            verticalCenterOffset: 39
        }
        color:"white"
        font.family: clockFont.name
        font.preferShaping: true
        font.pixelSize:20
        font.kerning: true
        style:Text.Outline
        styleColor: "#000000"
        layer.enabled: true
        layer.effect: InnerShadow{
            opacity:1
            spread:.7
            samples:8
            color:"black"
            cached: false
        }
    }
    Image{
        id:fanIcon
        cache:false
        height:24
        width:24
        anchors{
            verticalCenter: fanRPM.verticalCenter
            verticalCenterOffset: -2
            left:parent.left
            leftMargin:88
        }
        source:"fan.svg"
    }
    Text{
        id: rpmLabel
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors{
            verticalCenter:fanRPM.verticalCenter
            verticalCenterOffset: 16
            left:parent.left
            leftMargin:82
        }
        color:"#c2b2b2"
        font.pixelSize: 10
        font.letterSpacing: 1
        font.family: clockFont.name
        style:Text.Outline
        styleColor: "#000000"
        layer.enabled: true
        layer.effect: InnerShadow{
            radius:1
            spread:.6
            samples:8
            color:"black"
            cached: false
        }
        text:"RPM"
    }
    Text{
        id: pumpDuty
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors{
            verticalCenter: parent.verticalCenter
            left:parent.left
            leftMargin:38
            verticalCenterOffset: 78
        }
        color:"white"
        font.family: clockFont.name
        font.letterSpacing: 0
        font.preferShaping: true
        font.pixelSize:14
        font.kerning: true
        style:Text.Outline
        styleColor: "#000000"
        layer.enabled: true
        layer.effect: InnerShadow{
            opacity:1
            spread:.7
            samples:8
            color:"black"
            cached: false
        }
    }
    Image{
        id:pumpIcon
        cache:false
        height:32
        width:32
        anchors{
            verticalCenter: pumpDuty.verticalCenter
            left:parent.left
            leftMargin:68
        }
        source:"water-pump.svg"
    }

    Text{
        id:compassText
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors{
            verticalCenter: parent.verticalCenter
            left:parent.left
            leftMargin:24
            verticalCenterOffset: 70
        }
        color:"#e34aee"
        font.family: clockFont.name
        font.preferShaping: true
        font.pixelSize:12
        font.kerning: true
        style:Text.Outline
        styleColor: "#000000"
        layer.enabled: true
        layer.effect: InnerShadow{
            opacity:1
            spread:.7
            samples:8
            color:"black"
            cached: false
        }
        text:"N"
    }
    Text{
        id: dutyLabel
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors{
            horizontalCenter: pumpIcon.horizontalCenter
            horizontalCenterOffset: -8
            verticalCenter:parent.verticalCenter
            verticalCenterOffset:96
        }
        color:"#c2b2b2"
        font.pixelSize: 8
        font.letterSpacing: 0
        font.family: clockFont.name
        style:Text.Outline
        styleColor: "#000000"
        layer.enabled: true
        layer.effect: InnerShadow{
            radius:.5
            spread:.4
            samples:8
            color:"black"
            cached: false
        }
        text:"DUTY"
    }

    Rectangle{
        color:"black"
        radius:40
        width:80
        height:80
        anchors{
            bottom:parent.bottom
            horizontalCenter:parent.horizontalCenter
            horizontalCenterOffset: -24
            bottomMargin:-20
        }
        AnimatedImage{
            id:globe
            width:64
            height:64
            source:"globe.gif"
            speed:0
            currentFrame:0
            anchors.centerIn: parent
            visible:false
        }
        Rectangle{
            id: previewCircle
            color:"black"
            anchors.fill: parent
            anchors.margins: 4
            border.width: 2
            radius:width
            border.color: "#cccccc"
            visible: true
        }
        OpacityMask{
            id:opacityMask
            maskSource:previewCircle
            source: globe
            anchors.centerIn: parent
            height:64
            width:64
        }
    }

    Text{
        id: minuteText
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        anchors{
            horizontalCenter: parent.horizontalCenter
            verticalCenter:parent.verticalCenter
            verticalCenterOffset: 60
            horizontalCenterOffset:32
        }
        color:"#ff8800"
        font.family: numberFont.name
        font.letterSpacing: 2
        font.pixelSize: 92
        font.bold: true
        style:Text.Outline
        styleColor: "#000000"
        layer.enabled: true
        layer.effect:  InnerShadow{
            opacity:1
            anchors.fill: minuteText
            radius:6
            spread:.75
            samples:32
            color:"black"
            cached: false
            source:minuteText

          }

    }

    Text {
        id: amLabel
        color:top.isAm ? "white":"#5c5a5a"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 13
        font.letterSpacing: 1
        anchors{
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
            horizontalCenterOffset: 0;
            verticalCenterOffset: 0;
        }
        font.bold:true
        font.family: clockFont.name
        style:Text.Outline
        styleColor: "#000000"
        layer.enabled: true
        layer.effect: InnerShadow{
            opacity:1
            spread:.7
            samples:8
            color:"black"
            cached: false
          }
        text:"AM"
    }
    Text {
        id: pmLabel
        color: top.isAm ? "#5c5a5a":"white"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 13
        font.letterSpacing: 1
        anchors{
            horizontalCenter: parent.horizontalCenter
            verticalCenter: amLabel.verticalCenter
            horizontalCenterOffset: 34;
        }
        font.bold:true
        font.family: clockFont.name
        text:"PM"
        style:Text.Outline
        styleColor: "#000000"
        layer.enabled: true
        layer.effect: InnerShadow{
            opacity:1
            spread:.7
            samples:8
            color:"black"
            cached: false
          }
    }
    Text {
        id: tfHourLabel
        color: top.is24Hour ? "white":"#5c5a5a"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 13
        font.letterSpacing: 1
        anchors{
            horizontalCenter: parent.horizontalCenter
            verticalCenter: amLabel.verticalCenter
            horizontalCenterOffset: 72;
        }
        font.bold:true
        font.family: clockFont.name
        text:"H24"
        style:Text.Outline
        styleColor: "#000000"
        layer.enabled: true
        layer.effect: InnerShadow{
            opacity:1
            spread:.7
            samples:8
            color:"black"
            cached: false
          }
    }
    Component.onCompleted: {
        top.setArrays(Qt.locale().name);
        top.setCompassText(KrakenZDriver.rotationOffset);
        AppController.setFrameDelay(920);
        const options = { weekday: 'long', day: 'numeric' };
        var date = new Date();
        var hours = date.getHours() + top.hoursPassed;
        top.lastHour = hours;
        globe.currentFrame = (80 + (hours * 6)) % 180; // update the globe
        if(top.is24Hour) {  // set values for 24 hour clock
            top.currentHour = hours;
            top.isAm = false;
        } else {
            if(hours >= 12) {
                top.isAm = false;
                if(hours > 12){
                    hours -= 12;
                }
            }else {
                top.isAm = true;
                if(hours == 0) {
                    hours = 12;
                }
            }
        }
        hourText.text = hours.toString().padStart(2, '0')
        // an hour went by? maybe it is a new day!
        top.dateText = top.weekDays[date.getDay()] + '  ' + date.getDate().toString().padStart(2,'0');
        monthText.text = top.months[date.getMonth()]
        yearText.text = date.getFullYear()
        minuteText.text = date.getMinutes().toString().padStart(2, '0')
        secondsText.text = date.getSeconds().toString().padStart(2, '0')
        fanRPM.text = KrakenZDriver.fanSpeed
        pumpDuty.text = KrakenZDriver.pumpDuty
    }

}
