import QtQuick 2.15
import QtGraphicalEffects 1.15

Rectangle {
    id:background
    property alias sourceImage: sourceImage
    color:"transparent"
    height:320
    width:320
    property alias playing: sourceImage.playing
    property alias currentFrame: sourceImage.currentFrame
    property alias frameCount: sourceImage.frameCount
    property alias renderTarget: previewCircle
    AnimatedImage{
        id: sourceImage
        smooth: true
        visible: false
        fillMode: Image.PreserveAspectCrop
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
        source:sourceImage
        anchors.centerIn: parent
        height:320
        width:320
    }
}
