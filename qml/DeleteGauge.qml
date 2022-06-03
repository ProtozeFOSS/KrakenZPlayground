import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id:gaugeTop
    color:"transparent"
    property alias percentage:deleteSlider.value
    signal deleteReached();
    Slider{
        id:deleteSlider
        anchors.fill: parent
        live:true
        value:0
        from:0
        to:100
        handle:Rectangle{
            Rectangle{
                anchors.centerIn: parent
                height:32
                width:3
                radius:4
                color:"black"
            }

            color: "#ceccd1"
            border.color: "#161616"
            border.width: 2
            height: 42
            width: 54
            radius:6
            x: deleteSlider.leftPadding + deleteSlider.visualPosition * (deleteSlider.availableWidth - width)
            y: deleteSlider.topPadding + deleteSlider.availableHeight / 2 - height / 2
        }
        background:Rectangle{
            id:innerArea
            anchors.fill: parent
            anchors.margins: 8
            color:deleteSlider.value < 96 ? "#c0c0c0":"red"
            radius:8
            Text{
                anchors.fill: parent
                font.pixelSize: parent.height*.75
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text:"DELETE"
                font.bold: true
                color:"white"
                style: Text.Sunken
            }
        }
        onPressedChanged: {
            if(!pressed) {
                if(deleteSlider.value < 96) {
                    letGo.start();
                }else {
                    gaugeTop.deleteReached();
                    letGo.start();
                }
            }
        }

        stepSize:1
    }
    NumberAnimation{
        id:letGo
        target:deleteSlider
        property:"value"
        duration:100
        to:0
        alwaysRunToEnd: true;
    }

}
