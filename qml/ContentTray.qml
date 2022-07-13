import QtQuick 2.15

Rectangle
{
    id: trayTop
    property var contentObject:undefined
    signal deleteContent();
    onContentObjectChanged: {
        if(contentObject) {
            icon.source = contentObject.icon
            contentName.text = contentObject.name;
            contentLabel.text = contentObject.entry ? contentObject.entry.replace(contentObject.path,""):contentObject.icon.replace(contentObject.path,"")
        }
    }

    border.width: 2
    border.color: "white"
    color: "#4d4d4d"
    radius:8

    Text{
        id:contentName
        anchors{
            left:parent.left
            top: parent.top
            topMargin:8
            leftMargin:100
        }
        color:"white"
        font.pixelSize: 28
        font.bold: true
        style:Text.Sunken
        styleColor:"black"
    }
    Rectangle{
        id:folderButton
        color: "transparent"
        height:34
        width:34
        anchors.verticalCenter:contentName.verticalCenter
        anchors.left: contentName.right
        anchors.leftMargin: 12
        Image{
            height:32
            width:32
            anchors.centerIn: parent
            antialiasing: true
            smooth: true
            source:"qrc:/images/folder.png"
        }
        MouseArea{
            anchors.fill: parent
            onClicked:{
                Modules.openExplorerAt(trayTop.contentObject.path);
            }
        }
    }

    SliderGauge{
        id:deleteGauge
        anchors{
            right:parent.right
            verticalCenter: contentName.verticalCenter
            rightMargin:8
        }
        width: 200 <= (parent.width - contentName.anchors.leftMargin - contentName.paintedWidth - 24) ? 200 :(parent.width - contentName.anchors.leftMargin - contentName.paintedWidth - 24)
        onPercentageChanged: {
            let currentFrame = Math.floor((percentage/100) * 24);
            trashIcon.currentFrame = currentFrame;
            icon.setFrame(currentFrame);
        }
        innerText.text:"DELETE"
        onThresholdReached: trayTop.deleteContent()
        height: 42
    }
    Image{
        id:icon
        x:8
        y:4
        height:80
        width:80
        NumberAnimation{
            target:parent
            properties: "x,y,height,width,rotation"
            duration:100
        }

        fillMode:Image.PreserveAspectFit
        function setFrame(frame) {
            switch(frame) {
                case 0:{
                    icon.x = 8; icon.y = 4;
                    icon.rotation = 0;
                    icon.height = 80;
                    icon.width = 80;
                    break;
                }
                case 1:{
                    icon.x = 64; icon.y = 6;
                    icon.rotation = 2;
                    break;
                }
                case 3:{
                    icon.x = 120; icon.y = 8;
                    icon.rotation = 8;
                    break;
                }
                case 4:{
                    icon.x = 190; icon.y = 10;
                    icon.rotation = 10;
                    break;
                }
                case 5:{
                    icon.x = 270; icon.y = 13;
                    icon.rotation = 24;
                    icon.height = 76;
                    icon.width = 76;
                    break;
                }
                case 5:{
                    icon.x = 320; icon.y = 16;
                    icon.rotation = 36;
                    break;
                }
                case 6:{
                    icon.x = 340; icon.y = 18;
                    icon.rotation = 72;
                    break;
                }
                case 7:{
                    icon.x = 370; icon.y = 20;
                    icon.rotation = 136;
                    break;
                }
                case 8:{
                    icon.x = 380; icon.y = 24;
                    icon.rotation = 180;
                    break;
                }
                case 8:{
                    icon.x = 390; icon.y = 25;
                    icon.rotation = 240;
                    break;
                }
                case 9:{
                    icon.x = 410; icon.y = 26;
                    icon.rotation = 265;
                    break;
                }
                case 10:{
                    icon.x = 425; icon.y = 28;
                    icon.rotation = 285;
                    break;
                }
                case 11:{
                    icon.x = 430; icon.y = 31;
                    icon.rotation = 300;
                    break;
                }
                case 12:{
                    icon.x = 450; icon.y = 33;
                    icon.rotation = 320;
                    break;
                }
                case 13:{
                    icon.x = 460; icon.y = 34;
                    icon.rotation = 328;
                    break;
                }
                case 14:{
                    icon.x = 465; icon.y = 36;
                    icon.rotation = 336;
                    break;
                }
                case 15:{
                    icon.x = 467; icon.y = 41;
                    icon.rotation = 346;
                    break;
                }
                case 16:{
                    icon.x = 470; icon.y = 40;
                    icon.rotation = 352;
                    icon.height = 80;
                    icon.width = 80;
                    break;
                }
                case 17:{
                    icon.x = 472; icon.y = 42;
                    icon.rotation = 358;
                    icon.height = 82;
                    icon.width = 82;
                    break;
                }
                case 18:{
                    icon.x = 474; icon.y = 46;
                    icon.rotation = 360;
                    icon.height = 80;
                    icon.width = 80;
                    break;
                }
                case 19:{
                    icon.x = 477; icon.y = 45;
                    icon.rotation = 368;
                    icon.height = 76;
                    icon.width = 76;
                    break;
                }
                case 20:{
                    icon.x = 476; icon.y = 52;
                    icon.rotation = 362;
                    icon.height = 74;
                    icon.width = 74;
                    break;
                }
                case 21:{
                    icon.x = 475; icon.y = 64;
                    icon.rotation = 358;
                    icon.height = 70;
                    icon.width = 70;
                    break;
                }
                case 22:{
                    icon.y = 80;
                    icon.x = 476;
                    icon.rotation = 360;
                    break;
                }
            }
        }
    }

    AnimatedImage{
        id:trashIcon
        height:145
        fillMode:Image.PreserveAspectFit
        playing:false
        asynchronous: true
        anchors{
            right:parent.right
            top:parent.top
            margins:2
            topMargin:12
        }
        onCurrentFrameChanged: {
            switch(currentFrame) {
                case 0: {
                    trashIcon.height = 72
                    trashIcon.anchors.margins = -2;
                    trashIcon.anchors.topMargin = -12;
                    break;
                }
                case 1: {
                    trashIcon.anchors.margins = 3;
                    trashIcon.anchors.topMargin = 8;
                    trashIcon.height = 90
                    break;
                }
                case 2: {
                    trashIcon.anchors.margins = 4;
                    trashIcon.anchors.topMargin = 14;
                    trashIcon.height = 100
                    break;
                }
                case 3: {
                    trashIcon.anchors.margins = 5;
                    trashIcon.anchors.topMargin = 20;
                    trashIcon.height = 125
                    break;
                }
                case 4: {
                    trashIcon.anchors.margins = 6;
                    trashIcon.anchors.topMargin = 28;
                    trashIcon.height = 135
                    break;
                }
                case 5: {
                    trashIcon.anchors.margins = 7;
                    trashIcon.anchors.topMargin = 36;
                    trashIcon.height = 140
                    break;
                }
                default: {
                    trashIcon.height = 145
                    trashIcon.anchors.margins = 10;
                    trashIcon.anchors.topMargin = 40;
                }
            }
        }

        cache:true
        source:"qrc:/images/trash_catch.gif"
    }
    Text{
        id:contentLabel
        anchors{
            left:parent.left
            top:contentName.bottom
            topMargin:8
            leftMargin:110
        }
        color:"white"
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 18
    }
}
