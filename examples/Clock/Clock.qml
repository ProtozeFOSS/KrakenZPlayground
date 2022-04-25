import QtQuick 2.15

Item {
    id:top
    property int hours
    property int minutes
    property int seconds
    property alias rootItem:bg
    Connections {
        target: AppController
        function onDraw() {
            var date = new Date;
            top.hours = date.getHours()
            top.minutes = date.getMinutes()
            top.seconds = date.getSeconds();
        }
    }
    Rectangle{
        id : bg
        width:360
        height:360
        color:"transparent"
        anchors.centerIn: parent
        Image { anchors.centerIn: parent; width:bg.width; height:width; antialiasing: true; fillMode: Image.PreserveAspectFit; source: "clock-night.png"; visible: true }

        Image {
            id:hourHand
            x: (parent.width/2) - hourHand.width/2; y:(parent.height/2 - hourHand.height)
            source: "hour.png"
            height:parent.width * .25
            width:parent.width * .048
            transform: Rotation {
                id: hourRotation
                origin.x:hourHand.width/2; origin.y: hourHand.height
                angle: (top.hours * 30) + (top.minutes * 0.5)
                Behavior on angle {
                    SpringAnimation { spring: 2; damping: 0.2; modulus: 360 }
                }
            }
        }

        Image {
            id:minuteHand
            x: (parent.width/2) - minuteHand.width/2; y: (parent.height/2 - minuteHand.height)
            source: "minute.png"
            height:parent.width * .32
            width:parent.width * .038
            antialiasing:true
            transform: Rotation {
                id: minuteRotation
                origin.x:minuteHand.width/2; origin.y: minuteHand.height
                angle: (top.minutes * 6)
                Behavior on angle {
                    SpringAnimation { spring: 2; damping: 0.2; modulus: 360 }
                }
            }
        }

        Image {
            id:secondHand
            x: (parent.width/2) - secondHand.width/2; y: (parent.height/2 - secondHand.height)
            height:parent.width * .34
            width: parent.width * .0375
            source: "second.png"
            transform: Rotation {
                origin.x: secondHand.width/2; origin.y: secondHand.height;
                angle: (top.seconds * 6) + 2
                Behavior on angle {
                    SpringAnimation { spring: 2; damping: 0.2; modulus: 360 }
                }
            }
        }

        Image {
            anchors.centerIn: parent; source: "center.png"
            width:parent.height * .1
            fillMode: Image.PreserveAspectFit
            antialiasing: true;
            height:width
        }
    }

    Component.onCompleted: {
        AppController.setFrameDelay(400);
    }
}
