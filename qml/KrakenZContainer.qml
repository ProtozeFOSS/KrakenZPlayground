import QtQuick 2.15
import com.application.kzp 1.0
Rectangle {
    id: background
    color:"black"
    Connections{
        target:AppController
        function onModeChanged(mode) {
            switch(mode) {
                case OffscreenApp.GIF_MODE: {
                    AppController.animationPlaying = true;
                    break;
                }
                default:{
                    if(mode === OffscreenApp.QML_APP){
                        AppController.renderNext();
                    }
                    break;
                }
            }
        }
    }
    x:-1
    y:-1
    width:322;
    height:322;
    rotation:-1*KrakenZDriver.rotationOffset
    Rectangle{
        id:appContainer
        color:"black"
        anchors.centerIn:parent
        width:320
        height:320
        visible:AppController.mode === OffscreenApp.QML_APP
    }

    AnimatedImage{
        id: animatedImage
        smooth: true
        visible: AppController.mode == OffscreenApp.GIF_MODE
        cache: false
        fillMode: Image.PreserveAspectCrop
        source:AppController.mode == OffscreenApp.GIF_MODE ?  AppController.loadedPath:""
        playing:AppController.animationPlaying && animatedImage.visible
        anchors.centerIn: parent
        width:320
        height:320
        onFrameChanged:{
            AppController.renderNext();
        }
    }
    Image{
        id: image
        smooth: true
        visible: AppController.mode == OffscreenApp.STATIC_IMAGE
        cache: false
        fillMode: Image.PreserveAspectCrop
        source:AppController.mode == OffscreenApp.STATIC_IMAGE ?  AppController.loadedPath:""
        anchors.centerIn: parent
        width:320
        height:320
    }
    Rectangle{
        id: fpsDisplay
        visible:AppController.drawFPS
        anchors.bottom:parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        height: 28
        color: "#120084"
        width: 110
        radius:6
        gradient: Gradient {
            GradientStop {
                position: 0.00;
                color: "#0000ff";
            }
            GradientStop {
                position: 0.95;
                color: "#161949";
            }
        }
        Text{
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 18
            style: Text.Sunken
            styleColor: "#01767a"
            color:"white"
            text: "FPS: " + KrakenZDriver.fps.toString().slice(0,5)
            font.family: "Comic Sans MS"
        }
    }
}
