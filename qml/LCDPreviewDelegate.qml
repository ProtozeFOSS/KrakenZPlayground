import QtQuick 2.15
import QtGraphicalEffects 1.15

Rectangle {
    id:background
    Connections{
        target: KrakenImageProvider
        function onImageReady(){
            sourceImage.source = "image://krakenz/buffer" + Math.random(100).toString()
        }
    }

    property alias sourceImage: sourceImage
    color:"transparent"
    height:320
    width:320
    property alias renderTarget: previewCircle
    Image{
        id: sourceImage
        smooth: false
        visible: false
        asynchronous: true
        cache: false
        fillMode: Image.PreserveAspectCrop
        source:"image://krakenz/buffer"
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
        rotation:KrakenZDriver.rotationOffset
        maskSource:previewCircle
        source: sourceImage
        anchors.centerIn: parent
        height:320
        width:320
    }
}
