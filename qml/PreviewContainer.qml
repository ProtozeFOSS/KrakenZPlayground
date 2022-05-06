import QtQuick 2.15
import com.application.kzp 1.0

Item {
    id: root
    property bool isDetached : true
    property bool showFPS : false
    property alias contentItem: loader.item
    property int  radius: 0
    property int  brightness: 50
    onIsDetachedChanged: {
        previewWindow.active = root.isDetached;
    }

    onBrightnessChanged: {
        if(loader.item && loader.item.lens) {
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
        sourceComponent: root.isDetached ? empty : previewComponent
    }
    Loader{
        id:previewWindow
        active: false
        sourceComponent: PreviewWindow{ visible:false}
        onStatusChanged: {
            if(status == Loader.Ready) {
                //load settings
                previewWindow.item.visible = true;
                KZP.setPreviewWindow(previewWindow.item);
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
        Item{
            anchors.centerIn: parent
            width:root.width
            height:root.height
            property alias lens: lensItem
            LCDPreviewDelegate{
                id:krakenPreview
                anchors.fill: parent
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
                    visible: root.showFPS && (AppController.mode > OffscreenApp.STATIC_IMAGE)
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
            Rectangle{
                id:builtinMode
                visible: AppController.mode == OffscreenApp.BUILT_IN
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
                    text:"OEM\nDisplay Mode"
                    style:Text.Sunken
                    color:"white"
                }
            }
        }
    }
    Component.onCompleted: {
        previewWindow.active = root.isDetached;
    }
}
