import QtQuick 2.15

Rectangle {
    id : clock
    anchors.fill:parent
    property int hours
    property int minutes
    property int seconds

    function timeChanged() {
        var date = new Date;
        hours = date.getHours()
        minutes = date.getMinutes()
        seconds = date.getSeconds();
    }
    anchors.centerIn: parent
    width:340
    height:340
    color:"black"
    Timer {
        interval: 100; running: true; repeat: true;
        onTriggered: clock.timeChanged()
    }
    Image { anchors.centerIn: parent; width:400; height:364; fillMode: Image.PreserveAspectFit; source: "clock-night.png"; visible: true }

    Image {
        x: 148; y: 54
        source: "hour.png"
        height: 108
        width: 24
        transform: Rotation {
            id: hourRotation
            origin.x: 12; origin.y: 108;
            angle: (clock.hours * 30) + (clock.minutes * 0.5)
            Behavior on angle {
                SpringAnimation { spring: 2; damping: 0.2; modulus: 360 }
            }
        }
    }

    Image {
        x: 152; y: 31
        source: "minute.png"
        height: 132
        width:16
        transform: Rotation {
            id: minuteRotation
            origin.x: 8; origin.y: 132;
            angle: clock.minutes * 6
            Behavior on angle {
                SpringAnimation { spring: 2; damping: 0.2; modulus: 360 }
            }
        }
    }

    Image {
        x: 154; y: 13
        height: 150
        width: 12
        fillMode:Image.PreserveAspectFit
        source: "second.png"
        transform: Rotation {
            origin.x: 6; origin.y: 150;
            angle: clock.seconds * 6
            Behavior on angle {
                SpringAnimation { spring: 2; damping: 0.2; modulus: 360 }
            }
        }
    }

    Image {
        anchors.centerIn: parent; source: "center.png"
        width:40
        fillMode: Image.PreserveAspectFit
        height:width
    }

}
