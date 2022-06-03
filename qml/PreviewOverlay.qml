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
        height:32
        width:32
        Image{
            height:30
            width:30
            anchors.centerIn: parent
            antialiasing: true
            smooth: true
            source: "qrc:/images/download.svg"

        }
        MouseArea{
            anchors.fill: parent
            onClicked:{                
                if(AppController.mode === OffscreenApp.GIF_MODE) {
                    AppController.animationPlaying = false;
                }
                Preview.detachPreview(false);
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

        color:Preview.movementLocked ? "#252429":"red"
        height:32
        width:32
        Image{
            height:30
            width:30
            anchors.centerIn: parent
            antialiasing: true
            smooth: true
            source: Preview.movementLocked ? "qrc:/images/unlock.svg":"qrc:/images/lock.svg"
        }
        MouseArea{
            anchors.fill: parent
            onClicked:{
                Preview.lockMovement(!Preview.movementLocked);
                root.restartHideTimer()
            }
        }
    }

//    Rectangle{ // Toggle application settings, if they exist
//        radius:2
//        anchors{
//            top:parent.top
//            left:parent.left
//            margins:1
//        }
//        visible:Preview.hasSettings
//        enabled:visible
//        color: "#252429"
//        height:32
//        width:32
//        Image{
//            height:30
//            width:30
//            anchors.centerIn: parent
//            antialiasing: true
//            smooth: true
//            source:"qrc:/images/settings.svg"
//        }
//        MouseArea{
//            anchors.fill: parent
//            onClicked:{
//                Preview.showSettings(!Preview.settingsOpen);
//                root.restartHideTimer()
//            }
//        }
//    }

    Rectangle{ // Play and pause button
        radius:2
        anchors{
            bottom:parent.bottom
            left:parent.left
            margins:1
        }
        color: "#252429"
        height:32
        width:32
        visible:AppController.mode === OffscreenApp.GIF_MODE
        Image{
            height: 30
            width:30
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
