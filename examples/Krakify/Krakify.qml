import QtQuick 2.15
import QtQuick.Shapes 1.12
import QtMultimedia 5.8
import "RadialBar_arunpkqt"
Image{
    id:background
    anchors.fill: parent
    fillMode: Image.Stretch
    source:"Fly.png"

    Audio{
        id: song
        source:"Fly.mp3"
        loops: 0
        volume: .6
        onPositionChanged:{
            var remainder = song.duration-song.position;
            var milliseconds =  Math.round((remainder % 60000));
            var minutes =  (remainder-milliseconds)/60000;
            var seconds =  Math.round(milliseconds/1000);
            timeLeft.text = "-" + minutes + ":" + String(seconds).padStart(2,"0");

        }
        onStopped:{
            song.seek(0);
            song.play();
        }
    }
    AnimatedImage{
        playing:true
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
        height:320
        anchors.left:parent.left
        anchors.leftMargin: 176
        anchors.verticalCenterOffset: 1
        source:"orb-fly.gif"
    }

    Rectangle{
        color: "#33000000"
        anchors.fill: parent
    }
    Rectangle{
        id: headphonesIcon
        width:48
        height:48
        color: "#8024a3f2"
        radius:48
        anchors{
            top:parent.top
            topMargin:4
            horizontalCenter: parent.horizontalCenter
        }
        Image{
            source:"headphones.png"
            anchors.fill: parent
            anchors.margins:4
            fillMode: Image.PreserveAspectFit
        }
    }
    Text{
        id:albumLabel
        color: "#c6c6c6"
        font.family: "Arial"
        font.pixelSize: 9
        anchors{
            topMargin:2
            top: headphonesIcon.bottom
            horizontalCenter: parent.horizontalCenter
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text:"PLAYING FROM ALBUM"
    }
    Text{
        anchors{
            top: albumLabel.bottom
            horizontalCenter: parent.horizontalCenter
        }
        id:albumName
        font.family: "Arial"
        font.pixelSize: 10
        text:"NCS MUSIC"
        color: "#c6c6c6"
        font.bold:true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    Rectangle{
        radius:2
        height:22
        color:"#00000000"
        width:timePlayed.paintedWidth + 8
        anchors{
            left:parent.left
            leftMargin:3
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: 36
        }
        Text{
            id: timePlayed
            font.pixelSize: 13
            font.bold: true
            anchors.fill:parent
            color: "#c6c6c6"
            text:String((song.position - (song.position%60000))/60000) + ":" + String(Math.round((song.position%60000)/1000)).padStart(2, '0')
            font.letterSpacing: 1
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Rectangle{
        radius:2
        height:22
        color:"#00000000"
        width:timeLeft.paintedWidth + 8
        anchors{
            right:parent.right
            rightMargin:3
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: 36
        }
        Text{
            id: timeLeft
            font.pixelSize: 13
            anchors.fill:parent
            font.bold: true
            color: "#c6c6c6"
            text:"-0:00"
            font.letterSpacing: 1
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Text{
        id: songTitle
        text: "Fly (feat. Parker Pohill)"
        font.pixelSize: 16
        font.bold: true
        font.family: "Comic Sans MS"
        horizontalAlignment: Text.AlignHCenter
        color:"white"
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 72
    }
    Text{
        id:artist
        anchors.top:songTitle.bottom
        anchors.horizontalCenter: songTitle.horizontalCenter
        font.pixelSize : 10
        font.weight: Font.Thin
        color:"#bbffffff"
        horizontalAlignment: Text.AlignHCenter
        text:"Fransis Derelle"

    }

    RadialBarShape{
        id: progress
        rotation:0
        x:0
        y:20
        width:320
        height:300
        startAngle: -110
        spanAngle:-140
        minValue:0
        maxValue:song.duration
        value: song.position
        dialWidth:16
        backgroundColor:"transparent"
        progressColor: "#AF50FD02"
        dialColor: "#88191414"
        dialType: RadialBarShape.DialType.NoDial
    }

    Component.onCompleted: {
        song.play();
    }

}
