import QtQuick 2.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15
import com.application.kzp 1.0

Window {
    id:previewWindow
    visible: true
    color:"transparent"
    flags: Qt.FramelessWindowHint |  Qt.WA_TranslucentBackground | Qt.WindowStaysOnBottomHint
    property int windowOffsetX:0
    property int windowOffsetY:0
    width: Preview.width;
    height: Preview.height;


    Component.onCompleted: {
        previewWindow.x = Preview.x;
        previewWindow.y = Preview.y;
        previewWindow.width = Preview.width;
        previewWindow.height = Preview.height;
        KZP.setPreviewWindow(this);
        //settingsMode.active = !(previewMode.active = !Preview.settingsOpen)
    }
    Connections{
        target:Preview
        function onSettingsToggled(open) {
            settingsMode.active = Preview.settingsOpen
            previewMode.active = !Preview.settingsOpen;
        }
    }

    onVisibilityChanged: {
        if(visibility === Window.Hidden) {
            Preview.showSettings(false);
        }
    }

    Component.onDestruction: {
        //Preview.setPosition(previewWindow.x - previewWindow.windowOffsetX, previewWindow.y - (previewWindow.windowOffsetY *2));
        Preview.showSettings(false);
    }
    Loader{
        id:settingsMode
        active:false
        anchors.fill: parent
        sourceComponent: Rectangle{
            anchors.fill: parent
            color:"transparent"

            function showOverlay() {
                previewOnTop.showOverlay();
            }

            Loader{
                active: false
                id:settingsLoader
                source:Preview.settingsPath
                anchors.fill: parent
                onItemChanged: {
                    if(item) {
                        previewWindow.width = item.width
                        previewWindow.height = item.height
                        if(item.windowXOffset !== undefined) {
                            previewWindow.windowOffsetX = item.windowXOffset;
                            previewWindow.x += item.windowXOffset;
                        }

                        if(item.windowYOffset !== undefined) {
                            previewWindow.windowOffsetY = item.windowYOffset;
                            previewWindow.y += item.windowYOffset;
                            previewOnTop.y = -(item.windowYOffset);
                        }
                        if(item.previewContainer) {
                            previewOnTop.parent = item.previewContainer;
                        }else {
                            previewOnTop.x = -(previewWindow.windowOffsetX);
                            previewOnTop.y = -(previewWindow.windowOffsetY)
                        }
                    }
                }
            }
            LCDPreview{
                id:previewOnTop
                anchors.fill: parent
            }
            Component.onCompleted: {
                console.log("Settings Created");
                settingsLoader.active = true
            }
        }
    }


    Loader {
        id:previewMode
        active:true
        anchors.fill:parent
        onItemChanged:{
            if(item) {
                previewWindow.width = Preview.width
                previewWindow.height = Preview.height
                previewWindow.x = Preview.x;
                previewWindow.y = Preview.y;
                previewWindow.windowOffsetX = 0;
                previewWindow.windowOffsetY = 0;
                item.visible = true;
            }
        }

        sourceComponent:LCDPreview{
            id:lcdPreview
            visible:false
            anchors.fill: parent
            x:0
            y:0
            function showOverlay() {
                if(!overlayLoader.active) {
                    overlayLoader.active = true;
                }
                hideOverlay.restart();
            }
            MouseArea
            {
                anchors.fill: parent
                anchors.margins: 36
                property int m_x : 0;
                property int m_y : 0;
                onPressed:
                {
                    if(previewMode.item) {
                        previewMode.item.showOverlay();
                    }
                    m_x = mouse.x;
                    m_y = mouse.y;
                }
                onPositionChanged:
                {
                    if(!Preview.movementLocked) {
                        previewWindow.x = previewWindow.x + mouse.x - m_x
                        previewWindow.y = previewWindow.y + mouse.y - m_y
                        previewWindow.raise();
                        Preview.setPosition(previewWindow.x, previewWindow.y);
                    }
                    lcdPreview.showOverlay();
                }
            }
            Loader{
                id:overlayLoader
                active:true
                anchors.fill: parent
                sourceComponent: PreviewOverlay{
                    id:previewOverlay
                    anchors.fill:parent
                    visible:true
                    onRestartHideTimer: {
                        if(!overlayLoader.active) {
                            overlayLoader.active = true;
                        }
                        hideOverlay.restart();
                    }
                }
            }

            Timer{
                id:hideOverlay
                interval:5000
                repeat:false
                onTriggered:{
                    overlayLoader.active = false;
                }
            }
            Component.onCompleted: {
                hideOverlay.start();
            }            
        }        
    }
}
