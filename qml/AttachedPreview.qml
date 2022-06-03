import QtQuick 2.15
import com.application.kzp 1.0

LCDPreview{
    anchors.centerIn: parent
//    Rectangle{ // Toggle application settings, if they exist
//        radius:8
//        anchors{
//            top:parent.top
//            left:parent.left
//            margins:8
//        }
//        visible:Preview.hasSettings
//        enabled:visible
//        color: "#252429"
//        height:36
//        width:36
//        Image{
//            height:32
//            width:32
//            anchors.centerIn: parent
//            antialiasing: true
//            smooth: true
//            source:"qrc:/images/settings.svg"
//        }
//        MouseArea{
//            anchors.fill: parent
//            onClicked:{
//                Preview.showSettings(true);
//            }
//        }
//    }

    Rectangle{ // Toggles Play/Pause GIF
        radius:8
        anchors{
            bottom:parent.bottom
            left:parent.left
            margins:8
        }
        color: "#252429"
        height:36
        width:36
        visible:AppController.mode === OffscreenApp.GIF_MODE
        Image{
            id: ppIcon
            height: 32
            width:32
            anchors.centerIn: parent
            source: AppController.animationPlaying ?  "qrc:/images/pause.svg" : "qrc:/images/play.svg"
            antialiasing: true
            smooth: true
        }
        MouseArea{
            anchors.fill: parent
            onClicked:{
                AppController.animationPlaying = !AppController.animationPlaying
            }
        }
    }
}
