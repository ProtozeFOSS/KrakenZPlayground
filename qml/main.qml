import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    id:window
    width: 600
    height:640
    visible: true
    maximumHeight:640
    maximumWidth: 600
    title: qsTr("KrakenZ Playground")

    KrakenZ{
        anchors.fill: parent
        onAdvancedChanged: {
            if(advanced){
                window.maximumWidth = 1040
                window.width = 1040
            } else {
                window.maximumWidth = 600
                window.width = 600
            }
        }
    }
}
