import QtQuick 2.15
import com.application.kzp 1.0

Item {
    id:root
    anchors.fill: parent
    signal restartHideTimer()
    Rectangle{ // Attach to main winodw, or enter background mode
        radius:2
        anchors{
            top:parent.top
            right:parent.right
            margins:1
        }

        color: "#252429"
        height:28
        width:28
        Image{
            height:24
            width:24
            anchors.centerIn: parent
            antialiasing: true
            smooth: true
            source: "qrc:/images/download.svg"

        }
        MouseArea{
            anchors.fill: parent
            onClicked:{
                KZP.detachPreview(false);
                delete this;
            }
        }
    }

    Rectangle{ // Attach to main winodw, or enter background mode
        radius:2
        anchors{
            bottom:parent.bottom
            right:parent.right
            margins:1
        }

        color:KZP.movementLocked ? "#252429":"red"
        height:28
        width:28
        Image{
            height:24
            width:24
            anchors.centerIn: parent
            antialiasing: true
            smooth: true
            source: KZP.movementLocked ? "qrc:/images/unlock.svg":"qrc:/images/lock.svg"
        }
        MouseArea{
            anchors.fill: parent
            onClicked:{
                KZP.lockMovement(!KZP.movementLocked);
                root.restartHideTimer()
            }
        }
    }

    Rectangle{ // Attach to main winodw, or enter background mode
        radius:2
        anchors{
            top:parent.top
            left:parent.left
            margins:1
        }
        visible:AppController.hasSettings
        enabled:visible
        color: "#252429"
        height:28
        width:28
        Image{
            height:18
            width:18
            anchors.centerIn: parent
            antialiasing: true
            smooth: true
            source:"qrc:/images/lock.svg"
        }
        MouseArea{
            anchors.fill: parent
            onClicked:{
                AppController.toggleSettings();
                root.restartHideTimer()
            }
        }
    }

    Rectangle{
        radius:2
        anchors{
            bottom:parent.bottom
            left:parent.left
            margins:1
        }
        color: "#a8a7a7"
        height:28
        width:28
        visible:AppController.mode === OffscreenApp.GIF_MODE
        Image{
            id: ppIcon
            height: 24
            width:24
            anchors.centerIn: parent
            source: AppController.animationPlaying ?  "qrc:/images/pause.svg" : "qrc:/images/play.svg"
            antialiasing: true
            smooth: true
        }
        MouseArea{
            anchors.fill: parent
            onClicked: {
                AppController.animationPlaying = !AppController.animationPlaying
                root.restartHideTimer()
            }
        }
    }
}
