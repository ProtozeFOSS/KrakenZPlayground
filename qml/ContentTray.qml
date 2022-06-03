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

    DeleteGauge{
        id:deleteGauge
        anchors{
            right:parent.right
            verticalCenter: contentName.verticalCenter
            rightMargin:20
        }
        width: 200 <= (parent.width - contentName.anchors.leftMargin - contentName.paintedWidth - 36) ? 200 :(parent.width - contentName.anchors.leftMargin - contentName.paintedWidth - 36)
        onPercentageChanged: {
            let currentFrame = Math.floor((percentage/100) * 24);
            trashIcon.currentFrame = currentFrame;
            icon.setFrame(currentFrame);
        }
        onDeleteReached:trayTop.deleteContent()
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
                    icon.x = 64; icon.y = 4;
                    icon.rotation = 2;
                    break;
                }
                case 3:{
                    icon.x = 120; icon.y = 5;
                    icon.rotation = 8;
                    break;
                }
                case 4:{
                    icon.x = 190; icon.y = 6;
                    icon.rotation = 10;
                    break;
                }
                case 5:{
                    icon.x = 270; icon.y = 7;
                    icon.rotation = 24;
                    icon.height = 76;
                    icon.width = 76;
                    break;
                }
                case 5:{
                    icon.x = 320; icon.y = 8;
                    icon.rotation = 36;
                    break;
                }
                case 6:{
                    icon.x = 340; icon.y = 9;
                    icon.rotation = 72;
                    break;
                }
                case 7:{
                    icon.x = 370; icon.y = 10;
                    icon.rotation = 136;
                    break;
                }
                case 8:{
                    icon.x = 380; icon.y = 11;
                    icon.rotation = 180;
                    break;
                }
                case 8:{
                    icon.x = 390; icon.y = 12;
                    icon.rotation = 240;
                    break;
                }
                case 9:{
                    icon.x = 410; icon.y = 13;
                    icon.rotation = 280;
                    break;
                }
                case 10:{
                    icon.x = 425; icon.y = 14;
                    icon.rotation = 300;
                    break;
                }
                case 11:{
                    icon.x = 430; icon.y = 15;
                    icon.rotation = 310;
                    break;
                }
                case 12:{
                    icon.x = 450; icon.y = 16;
                    icon.rotation = 320;
                    break;
                }
                case 13:{
                    icon.x = 460; icon.y = 19;
                    icon.rotation = 328;
                    break;
                }
                case 14:{
                    icon.x = 470; icon.y = 24;
                    icon.rotation = 336;
                    break;
                }
                case 15:{
                    icon.x = 471; icon.y = 28;
                    icon.rotation = 346;
                    break;
                }
                case 16:{
                    icon.x = 472; icon.y = 29;
                    icon.rotation = 352;
                    icon.height = 84;
                    icon.width = 84;
                    break;
                }
                case 17:{
                    icon.x = 476; icon.y = 27;
                    icon.rotation = 358;
                    icon.height = 88;
                    icon.width = 88;
                    break;
                }
                case 18:{
                    icon.x = 480; icon.y = 30;
                    icon.rotation = 360;
                    icon.height = 80;
                    icon.width = 80;
                    break;
                }
                case 19:{
                    icon.x = 478; icon.y = 34;
                    icon.rotation = 368;
                    icon.height = 76;
                    icon.width = 76;
                    break;
                }
                case 20:{
                    icon.x = 474; icon.y = 48;
                    icon.rotation = 362;
                    icon.height = 72;
                    icon.width = 72;
                    break;
                }
                case 21:{
                    icon.x = 474; icon.y = 64;
                    icon.rotation = 358;
                    icon.height = 64;
                    icon.width = 64;
                    break;
                }
                case 22:{
                    icon.y = 72;
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
            if(currentFrame == 0) {
                trashIcon.height = 72
                trashIcon.anchors.margins = -2;
                trashIcon.anchors.topMargin = -8;
            }else if(currentFrame == 1) {
                trashIcon.anchors.margins = 3;
                trashIcon.anchors.topMargin = 8;
                trashIcon.height = 90
            }else if(currentFrame == 2) {
                trashIcon.anchors.margins = 5;
                trashIcon.anchors.topMargin = 16;
                trashIcon.height = 120
            }else {
                trashIcon.height = 145
                trashIcon.anchors.margins = 10;
                trashIcon.anchors.topMargin = 24;
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
