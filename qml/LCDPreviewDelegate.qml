import QtQuick 2.15
import QtGraphicalEffects 1.15

Rectangle {
    id:background
    property int bufferIndex: 0
    Connections{
        target: KrakenImageProvider
        enabled: !background.animated
        function onImageReady(){
            bufferIndex = !bufferIndex; // double buffering
            if(bufferIndex){
                sourceImage1.source = "image://krakenz/buffer" + Math.random(100).toString()
            }else {
                sourceImage0.source = "image://krakenz/buffer" + Math.random(100).toString()
            }
        }
    }
    property alias sourceImage0: sourceImage0
    property alias sourceImage1: sourceImage1
    property alias renderTarget: previewCircle
    color:"transparent"
    height:320
    width:320
    Image{
        id: sourceImage0
        smooth: false
        visible: false
        asynchronous: true
        cache: true
        fillMode: Image.PreserveAspectCrop
        source:"image://krakenz/buffer"
        width:320
        height:320
        onStatusChanged: {
            if(status == Image.Ready){
                opacityMask.source = sourceImage0;
                previewCircle.color = "black"
            }
        }
    }
    Image{
        id: sourceImage1
        smooth: false
        visible: false
        asynchronous: true
        cache: true
        fillMode: Image.PreserveAspectCrop
        source:""
        width:320
        height:320
        onStatusChanged: {
            if(status == Image.Ready){
                opacityMask.source = sourceImage1;
                previewCircle.color = "black"
            }
        }
    }
    Rectangle{
        id: previewCircle
        color:"transparent"
        anchors.centerIn: parent
        height:320
        width:320
        radius:width
        visible: true
    }
    OpacityMask{
        id:opacityMask
        rotation:KrakenZDriver.rotationOffset
        maskSource:previewCircle
        anchors.centerIn: parent
        height:320
        width:320
    }
}
