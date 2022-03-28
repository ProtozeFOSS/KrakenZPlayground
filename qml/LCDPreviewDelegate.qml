import QtQuick 2.15
import QtGraphicalEffects 1.15

Rectangle {
    id:background
    property bool animated: false
    rotation:KrakenZDriver.rotationOffset
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

    onAnimatedChanged: {
        if(animated){
            opacityMask.source = animatedImage;
            sourceImage0.source = ""
            sourceImage1.source = ""
        } else {
            bufferIndex = 0;
            sourceImage0.source = "image://krakenz/buffer" + Math.random(100).toString()
        }
    }

    property alias sourceImage0: sourceImage0
    property alias sourceImage1: sourceImage1
    property alias animationImage: animatedImage
    color:"transparent"
    height:320
    width:320
    property alias currentFrame: animatedImage.currentFrame
    property alias frameCount: animatedImage.frameCount
    property alias renderTarget: previewCircle
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
            }
        }
    }
    AnimatedImage{
        id: animatedImage
        smooth: true
        visible: false
        cache: true
        fillMode: Image.PreserveAspectCrop
        source:""
        width:320
        height:320
    }

    Rectangle{
        id: previewCircle
        color:"black"
        anchors.centerIn: parent
        height:320
        width:320
        radius:width
        visible: true
    }
    OpacityMask{
        id:opacityMask
        maskSource:previewCircle
        source: sourceImage0
        anchors.centerIn: parent
        height:320
        width:320
    }
}
