import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id:gaugeTop
    color:"transparent"
    property alias percentage:gaugeSlider.value
    property alias innerText:innerText
    property real threshold:96
    property alias slider:gaugeSlider
    signal thresholdReached();
    Slider{
        id:gaugeSlider
        anchors.fill: parent
        live:true
        value:0
        from:0
        to:100
        touchDragThreshold: .1
        snapMode: Slider.NoSnap
        handle:Rectangle{
            id:slider
            Behavior on x{
                NumberAnimation{
                    duration:20;
                }
            }
            color: "#b3544f4f"
            border.color: "#771f1f"
            border.width: 4
            height: 38
            width: 44
            radius:6
            x: gaugeSlider.leftPadding + gaugeSlider.visualPosition * (gaugeSlider.availableWidth - width)
            y: gaugeSlider.topPadding + gaugeSlider.availableHeight / 2 - height / 2
            Rectangle{
                height:24
                color: "#99ff0000"
                width:22
                radius:6
                anchors.centerIn: parent
                Image {
                    anchors.centerIn: parent
                    height:20
                    fillMode:Image.PreserveAspectFit
                    source:"qrc:/images/trash-2.svg"
                }
            }


        }
        background:Rectangle{
            id:innerArea
            anchors.fill: parent
            anchors.margins: 8
            anchors.topMargin: 4
            anchors.bottomMargin: 4
            color:gaugeSlider.value < 96 ? "#975252":"red"
            radius:6
            Text{
                id:innerText
                anchors.fill: parent
                font.pixelSize: parent.height*.75
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                font.bold: true
                color:"white"
                style: Text.Sunken
            }
        }

        MouseArea{
            preventStealing: true
            anchors{
                left:slider.right
                top:parent.top
                bottom:parent.bottom
                right:parent.right
            }
            onClicked:{}
        }
        onPressedChanged: {
            if(!pressed) {
                if(gaugeSlider.value < gaugeTop.threshold) {
                    letGo.start();
                }else {
                    gaugeTop.thresholdReached();
                    letGo.start();
                }
            }
        }

        stepSize:1
    }
    NumberAnimation{
        id:letGo
        target:gaugeSlider
        property:"value"
        duration:100
        to:0
        alwaysRunToEnd: true;
    }

}
