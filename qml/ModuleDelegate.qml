import QtQuick 2.15

Item{
    property alias moduleLabel:moduleName
    property alias color:iconRect.color
    Rectangle{
        id:iconRect
        anchors.fill:parent
        anchors.bottomMargin: 24
        color: "#698295"
        radius:8
        Image{
            fillMode:Image.PreserveAspectFit
            anchors.fill: parent
            anchors.margins: 2
            source:model.modelData.icon
        }
    }
    Text{
        id:moduleName
        font.pixelSize: 12
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top:iconRect.bottom
        anchors.bottom: parent.bottom
        text:model.modelData.name
    }
}
