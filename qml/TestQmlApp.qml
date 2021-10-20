import QtQuick 2.15

Item {
    Rectangle{
        color:"yellow"
        anchors.margins: 24
        anchors.fill: parent
    }

    Rectangle{
        id:spinner
        color:"red"
        border.width:2
        border.color: "black"
        radius:2
        anchors.centerIn: parent
        width:parent.width - 64
        height:parent.height - 64
    }
    Timer{
        running:true
        repeat: true
        interval: 12
        onTriggered: {
            spinner.rotation += 3
        }
    }
}
