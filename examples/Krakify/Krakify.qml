import QtQuick 2.15
import QtQuick.Shapes 1.12
import QtMultimedia 5.8

Image{
    id:background
    anchors.centerIn: parent
    height:330
    width:320
    fillMode: Image.Stretch
    source:"Fly.png"

    Audio{
        id: song
        source:"Fly.mp3"
        loops: 0
        volume: .6
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
        color:"#66000000"
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
            anchors.fill:parent
            color: "#c6c6c6"
            text:"0:45"
            font.letterSpacing: 1
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }


    Text{
        id: timeLeft
        anchors.left:parent.left
    }
    Shape{
        asynchronous: true
        width:320
        height:170
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 138
        ShapePath{
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin
            strokeWidth: 5
            strokeColor:  "#4d4d4d"
            fillColor: "transparent"
            startX:6
            startY:0
            PathArc{
                x:26
                y:0
                radiusX:12
                radiusY:16
                //useLargeArc: true
            }
            PathArc{
                direction: PathArc.Counterclockwise
                x:294
                y:0
                radiusX:152
                radiusY:186
                //useLargeArc: true
            }
            PathArc{
                x:320
                y:0
                radiusX:12
                radiusY:8
                //useLargeArc: true
            }
            PathArc{
                x:6
                y:0
                radiusX:170
                radiusY:160
                useLargeArc: true
            }
        }
    }


//    Image{
//        anchors.bottom:parent.bottom
//        anchors.bottomMargin: 7
//        height:120
//        anchors.horizontalCenter: parent.horizontalCenter
//        width:288
//        fillMode: Image.Stretch
//        source:"ring.png"
//    }

    Component.onCompleted: {
        song.play();
    }

}
