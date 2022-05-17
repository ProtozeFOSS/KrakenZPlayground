import QtQuick 2.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15
import com.application.kzp 1.0

Window {
    id:previewWindow
    width: 320
    height:320
    visible: true
    color:"transparent"
    transientParent: null
    flags: Qt.FramelessWindowHint |  Qt.WA_TranslucentBackground | Qt.WindowStaysOnBottomHint
    property int windowOffsetX:0
    property int windowOffsetY:0
    Connections{
        target:Preview
        function onSettingsToggled(toggled) {
            previewMode.active = !(settingsMode.active = toggled);
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
                active: true
                id:settingsLoader
                source:Preview.settingsPath
                onItemChanged: {
                    if(item) {
                        previewWindow.width = item.width
                        previewWindow.height = item.height
                        if(item.windowXOffset !== undefined) {
                            previewWindow.windowOffsetX = item.windowXOffset;
                            previewWindow.x += item.windowXOffset;
                            previewOnTop.x = -(item.windowXOffset);
                        }

                        if(item.windowYOffset !== undefined) {
                            previewWindow.windowOffsetY = item.windowYOffset;
                            previewWindow.y += item.windowYOffset;
                            previewOnTop.y = -(item.windowYOffset);
                        }
                    }
                }
            }
            LCDPreview{
                id:previewOnTop
                function showOverlay()
                {
                    if(!overlayLoader.active) {
                        overlayLoader.active = true;
                    }
                    hideOverlay.restart();
                }
                Loader{
                    id:overlayLoader
                    active:true
                    anchors.fill: previewOnTop.preview
                    sourceComponent: PreviewOverlay{
                        id:previewOverlay
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
                    Component.onCompleted: {
                       hideOverlay.start();
                    }
                }
            }
        }
    }


    Loader {
        id:previewMode
        active:true
        anchors.fill:parent
        onItemChanged:{
            if(item) {
                previewWindow.width = 320
                previewWindow.height = 320
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
            function showOverlay() {
                if(!overlayLoaderT.active) {
                    overlayLoaderT.active = true;
                }
                hideOverlayT.restart();
            }

            Loader{
                id:overlayLoaderT
                active:true
                anchors.fill: parent
                sourceComponent: PreviewOverlay{
                    id:previewOverlayT
                    anchors.fill:parent
                    visible:true
                    onRestartHideTimer: {
                        if(!overlayLoaderT.active) {
                            overlayLoaderT.active = true;
                        }
                        hideOverlayT.restart();
                    }
                }
            }

            Timer{
                id:hideOverlayT
                interval:5000
                repeat:false
                onTriggered:{
                    overlayLoaderT.active = false;
                }
            }
            Component.onCompleted: {
                hideOverlayT.start();
            }
        }
    }

    MouseArea
    {
        anchors.fill: parent
        property int m_x : 0;
        property int m_y : 0;
        propagateComposedEvents:true
        onPressed:
        {
            if(previewMode.item) {
                previewMode.item.showOverlay();
            }
            if(settingsMode.item){
                settingsMode.item.showOverlay();
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
                Preview.setPosition(previewWindow.x - previewWindow.windowOffsetX, previewWindow.y - previewWindow.windowOffsetY);
            }
            if(previewMode.item) {
                previewMode.item.showOverlay();
            }
            if(settingsMode.item){
                settingsMode.item.showOverlay();
            }
        }
    }

    Component.onCompleted: {
        previewWindow.x = Preview.x;
        previewWindow.y = Preview.y;
        KZP.setPreviewWindow(this);
        settingsMode.active = !(previewMode.active = !Preview.settingsOpen)
    }
}
