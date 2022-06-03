import QtQuick 2.15
import com.application.kzp 1.0

Item {
    id: root
    property alias contentItem: attachedPreview.item
    property int  radius: 0
    property int  brightness: 50
    property alias previewWindow:previewWindow.item
    signal detached(detached:bool)
    Connections{
        target: Preview
        function onDetachChanged(detach : bool) {
            if(detach !== previewWindow.active){
                previewWindow.active = detach;
                attachedPreview.active = !detach;
            }
            root.detached(detach);
        }
    }

    onBrightnessChanged: {
        if(attachedPreview.item) {
            if(brightness > 50){
                attachedPreview.item.lens.color = "white";
                attachedPreview.item.lens.opacity = brightness/100 - 0.85;
            } else {
                attachedPreview.item.lens.color = "black";
                if(brightness == 0){
                    attachedPreview.item.lens.opacity = 1.0;
                }else {
                    attachedPreview.item.lens.opacity = (50 - brightness)/100;
                }
            }
        }
    }


    Loader{
        id:attachedPreview
        active: false
        anchors.fill:parent
        sourceComponent: AttachedPreview{ anchors.fill:parent; brightness:root.brightness; visible:false}
        onStatusChanged: {
            if(status == Loader.Ready) {
                item.visible = true;
                if(AppController.mode === OffscreenApp.GIF_MODE) {
                    AppController.animationPlaying = true;
                }
            }
        }
    }
    Loader{
        id:previewWindow
        active: false
        sourceComponent: PreviewWindow{visible:false}
        onStatusChanged: {
            if(status == Loader.Ready) {
                previewWindow.item.visible = true;
                if(AppController.mode === OffscreenApp.GIF_MODE) {
                    AppController.animationPlaying = true;
                }
            }
        }
    }
    Rectangle{
        anchors.fill: attachedPreview
        radius:width
        color:"#22262b"
        border.width: 2
        border.color: "#5c5c5c"
        visible:previewWindow.active
        Text{
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 24
            font.family: "Comic Sans MS"
            text:"Detached Mode"
            style:Text.Sunken
            color:"white"
        }
    }
    Rectangle{ // Toggles detached Preview
        radius:8
        anchors{
            top:parent.top
            right:parent.right
            margins:8
        }
        color: "#252429"
        height:36
        width:36
        Image{
            height:32
            width:32
            anchors.centerIn: parent
            antialiasing: true
            smooth: true
            source:!AppController ? "qrc:/images/upload.svg": (Preview.detached ? "qrc:/images/download.svg":"qrc:/images/upload.svg")
        }
        MouseArea{
            anchors.fill: parent
            onClicked:{
                if(AppController.mode === OffscreenApp.GIF_MODE) {
                    AppController.animationPlaying = false;
                }
                Preview.detached = !Preview.detached
            }
        }
    }
    Component.onCompleted:{
        previewWindow.active = Preview.detached;
        attachedPreview.active = !Preview.detached;
    }
}
