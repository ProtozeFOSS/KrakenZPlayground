import QtQuick 2.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15
import com.application.kzp 1.0

Window {
    id:previewWindow
    width: 320
    height:320
    visible: true
    minimumHeight: 320
    minimumWidth:320
    maximumHeight:320
    maximumWidth: 320
    x:KZP.previewX;
    y:KZP.previewY;
    color:"transparent"
    transientParent: null
    property alias overlay: overlayLoader.item
    property alias brightness: lcdPreview.brightness
    property alias preview:lcdPreview
    flags: Qt.FramelessWindowHint |  Qt.WA_TranslucentBackground
    LCDPreview{
        id:lcdPreview
        anchors.fill: parent
        MouseArea
        {
            anchors.fill: parent
            property int m_x : 0;
            property int m_y : 0;

            onPressed:
            {
                if(!overlayLoader.active) {
                    overlayLoader.active = true;
                }
                hideOverlay.restart();
                m_x = mouse.x;
                m_y = mouse.y;
            }


            onPositionChanged:
            {
                if(!KZP.movementLocked) {
                    window.x = window.x + mouse.x - m_x
                    window.y = window.y + mouse.y - m_y
                    KZP.recordPreviewLocation(window.x, window.y);
                }
            }
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
        KZP.setPreviewWindow(this);
        previewWindow.flags += (KZP.state === KZPController.DETACHED ? Qt.WindowStaysOnBottomHint : Qt.WindowStaysOnTopHint)
    }
}
