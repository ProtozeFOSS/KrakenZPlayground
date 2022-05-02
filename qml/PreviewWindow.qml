import QtQuick 2.15
import QtQuick.Window 2.15
import QtGraphicalEffects 1.15
Window {
    id:window
    width: 320
    height:320
    visible: true
    minimumHeight: 320
    minimumWidth:320
    maximumHeight:320
    maximumWidth: 320
    color:"transparent"

    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnBottomHint | Qt.WA_TranslucentBackground
    LCDPreviewDelegate{
        id:krakenPreview
        color:"transparent"
        anchors.centerIn: parent
        width:320
        height:320
    }
    Item{
        id:imageOut
        anchors.centerIn: krakenPreview
        width:320
        height:320

        Rectangle{
            id: lens
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
            visible: true
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
        visible: false
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
    MouseArea
     {
         anchors.fill: parent

         property int m_x : 0;
         property int m_y : 0;

         onPressed:
         {
             m_x = mouse.x;
             m_y = mouse.y;
         }

         onPositionChanged:
         {
             window.x = window.x + mouse.x - m_x
             window.y = window.y + mouse.y - m_y
         }

     }
    Component.onCompleted: {
        AppController.loadQmlFile("file:/C:/Users/opopo/AppData/Local/Kraken Z Playground/examples/KZP_Clock/kzp_clock.qml")
    }
}
