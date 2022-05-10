import QtQuick 2.15
import com.application.kzp 1.0

Item{
    property int brightness:50
    anchors.centerIn: parent
    property alias lens: lensItem
    LCDPreviewDelegate{
        id:krakenPreview
        anchors.fill: parent
    }

    onBrightnessChanged: {
        if(brightness > 50){
            lens.color = "white";
            lens.opacity = brightness/100 - 0.85;
        } else {
            lens.color = "black";
            if(brightness == 0){
                lens.opacity = 1.0;
            }else {
                lens.opacity = (50 - brightness)/100;
            }
        }
    }
    Rectangle{
        id:builtinMode
        visible: AppController.mode == OffscreenApp.BUILT_IN || errorTitle.visible
        anchors.centerIn: krakenPreview
        width:320
        height:320
        color: "#22262b"
        radius:160
        border.width: 2
        border.color: "#5c5c5c"
        Text{
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 24
            font.family: "Comic Sans MS"
            text:errorTitle.visible ? "":"OEM\nDisplay Mode"
            style:Text.Sunken
            color:"white"
        }
    }
    Item{
        id:imageOut
        anchors.centerIn: krakenPreview
        width:320
        height:320

        Rectangle{
            id: lensItem
            anchors.fill: parent
            color:"black"
            radius:width
            opacity:0
        }

        Text{
            function reset(){
                errorTitle.visible = false;
                errorText.text = "";
            }
            id:errorTitle
            anchors.top:parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 16
            text:"QML Error"
            color:"lightblue"
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 24
            visible:false
        }

        Text{
            id:errorText
            anchors.centerIn:parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color:"white"
            width:280
            wrapMode:Text.WrapAnywhere
            visible:errorTitle.visible
        }
        Rectangle{
            id: fps
            visible: AppController.showFPS && (AppController.mode > OffscreenApp.STATIC_IMAGE)
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
    Connections{
        target: AppController
        function onAppReady(){
            errorTitle.reset();
        }
        function onModeChanged(mode) {
            errorTitle.reset();
        }

        function onQmlFailed(error) {
            errorText.text = error;
            errorTitle.visible = true;
        }
    }
}
