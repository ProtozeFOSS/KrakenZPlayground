import QtQuick 2.15

Rectangle {
    id: deviceContainer
    height:60 + sensorView.contentHeight
    width:120 // will expand to parent width
    clip:true
    property bool selected:false
    onSelectedChanged:{
        if(selected) {
            deviceContainer.state = "selected";
        } else {
            deviceContainer.state = "";
        }
    }

    states: [
        State{
            name:"" // default
            PropertyChanges{
                target: deviceContainer
                height: 80
            }
        },
        State{
            name:"selected"
            PropertyChanges{
                target: deviceContainer
                height: 300
            }
        }
    ]
    Text{
        id:deviceName
        color:"white"
        anchors{
            top:parent.top
            left:parent.left
            margins:2
            leftMargin:8
        }
        text:modelData.name
        font.pixelSize: 24
    }


    ListView{
        id:sensorView
        anchors{
            top:deviceName.bottom
            left:parent.left
            right:parent.right
            margins:2
        }
        height:contentHeight + 4
        model:modelData.sensors
        spacing:2
        delegate:SensorDelegate{width:sensorView.width}
    }

    Behavior on height {
        NumberAnimation{ duration:200 }
    }
    color: "#2c2f31"
}
