import QtQuick 2.15
import QtGraphicalEffects 1.15

Rectangle {
    id:background
    property bool animated: false
    onAnimatedChanged: {
        if(animated){
            opacityMask.source = animatedImage;
            sourceImage.source = ""
        } else {
            opacityMask.source = sourceImage
            sourceImage.source = "image://krakenz/buffer"
        }
    }

    property alias sourceImage: sourceImage
    property alias animationImage: animatedImage
    color:"transparent"
    height:320
    width:320
    property alias currentFrame: sourceImage.currentFrame
    property alias frameCount: sourceImage.frameCount
    property alias renderTarget: previewCircle
    Image{
        id: sourceImage
        smooth: true
        visible: false
        asynchronous: true
        cache: false
        fillMode: Image.PreserveAspectCrop
        source:""
        width:320
        height:320
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
        Text{
            visible: sourceImage.source.length === 0
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: "No Image Set"
            font.pixelSize: 24
            color:"white"
        }

    }
    OpacityMask{
        id:opacityMask
        maskSource:previewCircle
        source: animatedImage
        anchors.centerIn: parent
        height:320
        width:320
        //rotation: -KrakenZDriver.rotationOffset
    }
}
