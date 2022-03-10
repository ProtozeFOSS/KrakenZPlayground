import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    id:window
    width: 600
    height:720
    visible: true
    minimumHeight: 720
    minimumWidth:600
    maximumHeight:720
    maximumWidth: 600
    title: qsTr("Kraken Z Playground")
    Loader{
        anchors.fill: parent
        active:!userWarningLoader.active && KrakenZDriver.found
        sourceComponent: KrakenZ{
            onAdvancedChanged: {
                if(advanced){
                    window.maximumWidth = 1040
                    window.width = 1040
                    window.minimumWidth = 1040;
                } else {
                    window.minimumWidth = 600;
                    window.width = 600
                    window.maximumWidth = 600
                }
            }
            Component.onCompleted:{
                KrakenZDriver.setBrightness(55);
            }
        }
    }
    Loader{
        id:userWarningLoader
        active:true
        anchors.fill: parent
        sourceComponent:UserWarning{
            onAccepted:{
               if(KrakenZDriver.found){
                   KrakenZDriver.initialize();
               }
               userWarningLoader.active = false;
            }
        }
    }
}
