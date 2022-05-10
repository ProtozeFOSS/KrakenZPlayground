import QtQuick 2.15

Item {
    id: root
    property bool isDetached : true
    property alias contentItem: loader.item
    property int  radius: 0
    property int  brightness: 50
    property alias previewWindow:previewWindow.item
    onIsDetachedChanged: {
        detachTimer.start();
    }
    Timer{
        id:detachTimer
        interval:2
        running:false
        repeat:false
        onTriggered: {
            loader.active = false
            loader.sourceComponent = root.isDetached ? empty : previewComponent
            loader.active = true
            previewWindow.active = root.isDetached;
        }
    }
    onBrightnessChanged: {
        if(loader.item && !root.isDetached) {
            if(brightness > 50){
                loader.item.lens.color = "white";
                loader.item.lens.opacity = brightness/100 - 0.85;
            } else {
                loader.item.lens.color = "black";
                if(brightness == 0){
                    loader.item.lens.opacity = 1.0;
                }else {
                    loader.item.lens.opacity = (50 - brightness)/100;
                }
            }
        }
    }

    Loader{
        id:loader
        active:true
        sourceComponent: empty
    }
    Loader{
        id:previewWindow
        active: false
        sourceComponent: PreviewWindow{ brightness:root.brightness; visible:false}
        onStatusChanged: {
            if(status == Loader.Ready) {
                previewWindow.item.visible = true;
            }
        }
    }

    Component{
        id: empty
        Rectangle{
            anchors.centerIn: parent
            width:root.width
            height:root.height
            radius:root.radius
            color:"#22262b"
            border.width: 2
            border.color: "#5c5c5c"
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
    }
    Component{
        id: previewComponent
        LCDPreview{
            anchors.fill:parent
        }
    }
    Component.onCompleted: {
        detachTimer.start();
    }
}
