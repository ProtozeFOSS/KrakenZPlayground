import QtQuick 2.12
import QtQml.Models 2.12
Rectangle {
    id:advancedWindow
    width: 440
    height: 480
    visible: true
    color:"#4f4848"
    Connections{
        target:KrakenZDriver
        function onUsbMessage(message){
            itemList.append(message);
            if(itemList.length > 36){
                console.log("Deleting " + itemList.length - 36 + " items")
                itemList = itemList.slice(itemList.length-36,36);
            }
        }
    }

    // Main messaging window
    Rectangle{
        id: messageWindow
        color: "grey"
        border.width: 2
        anchors.top: parent.top
        anchors.left:parent.left
        anchors.right:parent.right
        anchors.bottom: inputBG.top
        anchors.bottomMargin: 4
        ListView{
            id:listView
            anchors.fill: parent
            anchors.margins: 4
            clip:true
            spacing:29
            delegate:Loader{
                height: item ?  item.paintedHeight:0
                width: listView.width - 64
                x:32
                sourceComponent: model.VALID ? validMessage : errorMessage;
                onItemChanged: {
                    if(item){
                        item.setItemData(model)
                    }
                }
            }
            model:ListModel{
                id:itemList
            }
        }
    }
    Rectangle{
        id:inputBG
        border.width: 1
        radius:2
        color:"white"
        height:24
        anchors.bottom:selectCat.top
        anchors.left:parent.left
        anchors.right:sendButton.left
        anchors.margins: 2
        TextInput{
            id: inputData
            leftPadding: 2
            anchors.fill: parent
            verticalAlignment: TextInput.AlignVCenter
            rightPadding:2
        }
    }
    Rectangle{
        id:selectCat
        anchors.bottom: parent.bottom
        anchors.left:parent.left
        anchors.margins: 2
        width:24
        height:24
        color:"blue"
        Image{
            anchors.fill: parent
            width:32
            height:32
            source:"qrc:/images/cat.png"
        }

        MouseArea{
            anchors.fill: parent
            onClicked:{
                KrakenZDriver.setImage(":/images/cat.png",0);
                //advancedWindow.imageChosen("qrc:/images/cat.png");
            }
        }
    }
    Rectangle{
        id:selectLogo
        anchors.bottom: parent.bottom
        anchors.left:selectCat.right
        anchors.margins: 2
        width:24
        height:24
        color:"red"

        MouseArea{
            anchors.fill: parent
            onClicked:{
                KrakenZDriver.setImage(":/images/NanCat.gif");
                //advancedWindow.imageChosen("qrc:/images/NanCat.gif");
            }
        }
    }

    Rectangle{
        id:selectIcon
        anchors.bottom: parent.bottom
        anchors.left:selectLogo.right
        anchors.margins: 2
        width:24
        height:24
        color:"yellow"
        Image{
            anchors.fill: parent
            width:32
            height:32
            source:"qrc:/images/Droplet.png"
        }

        MouseArea{
            anchors.fill: parent
            onClicked:{
                KrakenZDriver.setImage(":/images/Droplet.png");
                //advancedWindow.imageChosen("qrc:/TrayIcon2.png");
            }
        }
    }

    Rectangle{
        id:sendButton
        width:72
        border.width:2
        anchors.right:parent.right
        anchors.rightMargin: 2
        height:48
        color: "#336491"
        anchors.bottom:parent.bottom
        radius:4
        Text{
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text:"Send"
            color:"white"
        }
        MouseArea{
            anchors.fill: parent
            onClicked: KrakenZDriver.sendHex(inputData.text);
        }
    }

// Error Message
    Component {
        id: errorMessage
        Rectangle{ // Transmission message
            //height:innerText.paintedHeight + 16 > 64 ? innerText.paintedHeight + 16 : 64
            color:'lightblue'
            radius:12
            anchors.left: parent ?  parent.left : undefined
            anchors.leftMargin: 8
            height: 96
            function setItemData(itemData){
                target.text = itemData.TARGET;
                errorValue.text = itemData.ERROR ? data.ERROR:"BLAHH!! I said an index between 0 and 15 fool!";
            }
            Rectangle{
                id: cmdName
                color:"#585858"
                radius:4
                border.width: 1
                height: 22
                width: 356
                anchors{
                    top: parent.top
                    left: parent.left
                    topMargin: 4
                    leftMargin: 8
                }

                Text{
                    id: targetLabel
                    text:"Command:"
                    anchors{
                        left:parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: 2
                    }
                    font.bold: true
                    font.pixelSize: 10
                    color:"#a4a4a4"
                    width:34
                }
                TextEdit{
                    id:target
                    color:"white"
                    leftPadding: 4
                    rightPadding: 4
                    anchors.verticalCenter: targetLabel.verticalCenter
                    anchors.left:targetLabel.right
                    anchors.right:parent.right
                    font.pixelSize: 11
                    readOnly: true
                    overwriteMode: false
                    selectByMouse: true
                    horizontalAlignment: Text.Right
                    text:""
                }
            }
            Rectangle{
                id: errorAvatarBG
                anchors.left:parent.left
                anchors.top:parent.top
                anchors.leftMargin: 20
                anchors.topMargin:26
                width:64
                height: 64
                color: "#bb9428"
                radius: 4
                border.width: 1
                Image{
                    anchors.fill: parent
                    id:errorAvatar
                    source: "qrc:/SADaf.png"
                    height:64
                    width:height
                }
            }
            Text{
                id: errorValue
                anchors.left:parent.left
                anchors.top:parent.top
                anchors.leftMargin: 94
                anchors.topMargin:26
                width: 255
                lineHeight: 1.1
                font.pixelSize: 24
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
            Image{
                id: avatar
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -26
                fillMode: Image.PreserveAspectFit
                anchors.left: parent.right;
                anchors.right: undefined;
                anchors.leftMargin: -28;
                source: "qrc:/kraken_avatar.png";
                width:48
                height:width;
            }
        }
    }


//    VALID MESSAGE DEFINTION
    Component{
        id: validMessage
        Rectangle{ // Transmission message
            id:messageBox
            //height:innerText.paintedHeight + 16 > 64 ? innerText.paintedHeight + 16 : 64
            color: "#f6ddff"
            radius:12
            height: 154
            function setItemData(itemData){
                target.text = itemData.TARGET;
                raw.text = itemData.RAW
                cmdHexValue.text = "0x" + itemData.TYPE;
                messageBox.color = itemData.RECEIVED ? "lightblue":"lightgreen";
                bucketValue.text = itemData.BUCKET;
                assetValue.text = itemData.ASSET;
                modeValue.text = itemData.MODE;
                magicValue.text = itemData.SESSION;
                memorySlotsValue.text = "0x" + itemData.MEMORY_SLOTS;
                memoryStartValue.text = "0x" + itemData.MEMORY_START;
                if(itemData.RECEIVED == false){
                   avatar.setLeft();
                } else {
                    avatar.setRight();
                }

            }

            Rectangle{
                id: cmdName
                color:"#585858"
                radius:4
                border.width: 1
                height: 22
                width: 356
                anchors{
                    top: parent.top
                    left: parent.left
                    topMargin: 4
                    leftMargin: 8
                }

                Text{
                    id: targetLabel
                    text:"Command:"
                    anchors{
                        left:parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: 2
                    }
                    font.bold: true
                    font.pixelSize: 10
                    color:"#a4a4a4"
                    width:34
                }
                TextEdit{
                    id:target
                    color:"white"
                    leftPadding: 4
                    rightPadding: 4
                    anchors.verticalCenter: targetLabel.verticalCenter
                    anchors.left:targetLabel.right
                    anchors.right:parent.right
                    font.pixelSize: 11
                    readOnly: true
                    overwriteMode: false
                    selectByMouse: true
                    horizontalAlignment: Text.Right
                    text:""
                }
            }

            Rectangle{
                id: cmdHex
                color:"#585858"
                radius:4
                border.width: 1
                height: 22
                width: 106
                anchors{
                    top: cmdName.bottom
                    left: cmdName.left
                    topMargin: 4
                }

                Text{
                    id: cmdHexLabel
                    text:"CMD HEX:"
                    anchors{
                        left:parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: 2
                    }
                    font.bold: true
                    font.pixelSize: 10
                    color:"#a4a4a4"
                    width:34
                }
                TextEdit{
                    id:cmdHexValue
                    color:"#fbff7e"
                    leftPadding: 4
                    anchors.verticalCenter: cmdHexLabel.verticalCenter
                    anchors.left:cmdHexLabel.right
                    anchors.right:parent.right
                    font.pixelSize: 11
                    readOnly: true
                    overwriteMode: false
                    selectByMouse: true
                    horizontalAlignment: Text.AlignHCenter
                    text:""
                }
            }

            Rectangle{
                id: bucketBackground
                color:"#585858"
                radius:4
                border.width: 1
                height: 22
                width: 78
                anchors{
                    top: cmdHex.top
                    left: cmdHex.right
                    leftMargin: 4
                }

                Text{
                    id: bucketLabel
                    text:"Bucket:"
                    anchors{
                        left:parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: 2
                    }
                    font.bold: true
                    font.pixelSize: 10
                    color:"#a4a4a4"
                    width:34
                }
                TextEdit{
                    id:bucketValue
                    color:"#03ff0b"
                    leftPadding: 4
                    anchors.verticalCenter: bucketLabel.verticalCenter
                    anchors.left:bucketLabel.right
                    anchors.right:parent.right
                    font.pixelSize:11
                    readOnly: true
                    overwriteMode: false
                    selectByMouse: true
                    horizontalAlignment: Text.AlignHCenter
                    text:""
                }
            }
            Rectangle{
                id: assetBackground
                color:"#585858"
                radius:4
                border.width: 1
                height: 22
                width: 79
                anchors{
                    top: cmdHex.top
                    left: bucketBackground.right
                    leftMargin: 4
                }

                Text{
                    id: assetLabel
                    text:"Asset:"
                    anchors{
                        left:parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: 2
                    }
                    font.bold: true
                    font.pixelSize:10
                    color:"#a4a4a4"
                    width:34
                }
                TextEdit{
                    id:assetValue
                    color:"#16f7ff"
                    leftPadding: 4
                    anchors.verticalCenter: assetLabel.verticalCenter
                    anchors.left:assetLabel.right
                    anchors.right:parent.right
                    font.pixelSize:11
                    readOnly: true
                    overwriteMode: false
                    selectByMouse: true
                    horizontalAlignment: Text.AlignHCenter
                    text:""
                }
            }

            Rectangle{
                id: modeBackground
                color:"#585858"
                radius:4
                border.width: 1
                height: 22
                width: 79
                anchors{
                    top: cmdHex.top
                    left: assetBackground.right
                    leftMargin: 4
                }

                Text{
                    id: modeLabel
                    text:"Mode:"
                    anchors{
                        left:parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: 2
                    }
                    font.bold: true
                    font.pixelSize:10
                    color:"#a4a4a4"
                    width:34
                }
                TextEdit{
                    id:modeValue
                    color:"#ffd91a"
                    leftPadding: 4
                    anchors.verticalCenter: modeLabel.verticalCenter
                    anchors.left:modeLabel.right
                    anchors.right:parent.right
                    font.pixelSize:11
                    readOnly: true
                    overwriteMode: false
                    selectByMouse: true
                    horizontalAlignment: Text.AlignHCenter
                    text:""
                }
            }
            Rectangle{
                id: magicBackground
                color:"#585858"
                radius:6
                border.width: 1
                height: 32
                width: 352
                anchors{
                    left: cmdHex.left
                    top: cmdHex.bottom
                    topMargin: 4
                }
                Text{
                    anchors{
                        left:parent.left
                        top:parent.top
                        leftMargin: 4
                    }
                    text:"Magic # [2:14]"
                    font.bold: true
                    color:"#a4a4a4"
                    font.pixelSize: 13
                }

                TextEdit{
                    id:magicValue
                    color:"#bee3ff"
                    leftPadding: 4
                    rightPadding: 4
                    anchors.bottom:parent.bottom
                    anchors.left:parent.left
                    anchors.right:parent.right
                    font.pixelSize: 14
                    readOnly: true
                    overwriteMode: false
                    selectByMouse: true
                    horizontalAlignment: Text.Right
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text:""
                }
            }

            Rectangle{
                id: memStartBackground
                color:"#585858"
                radius:4
                border.width: 1
                height: 22
                width: 160
                anchors{
                    top: magicBackground.bottom
                    left: cmdHex.left
                    topMargin: 4
                    leftMargin:8
                }

                Text{
                    id: memoryStartLabel
                    text:"MEM Start:"
                    anchors{
                        left:parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: 2
                    }
                    font.bold: true
                    font.pixelSize:10
                    color:"#a4a4a4"
                    width:34
                }
                TextEdit{
                    id:memoryStartValue
                    color:"#d5b5ff"
                    leftPadding: 4
                    anchors.verticalCenter: memoryStartLabel.verticalCenter
                    anchors.left:memoryStartLabel.right
                    anchors.right:parent.right
                    font.pixelSize:11
                    readOnly: true
                    overwriteMode: false
                    selectByMouse: true
                    horizontalAlignment: Text.AlignHCenter
                    text:""
                }
            }

            Rectangle{
                id: memSlotsBackground
                color:"#585858"
                radius:4
                border.width: 1
                height: 22
                width: 160
                anchors{
                    top: memStartBackground.top
                    left: memStartBackground.right
                    leftMargin: 16
                }

                Text{
                    id: memSlotsLabel
                    text:"MEM Slots:"
                    anchors{
                        left:parent.left
                        verticalCenter: parent.verticalCenter
                        leftMargin: 2
                    }
                    font.bold: true
                    font.pixelSize:10
                    color:"#a4a4a4"
                    width:34
                }
                TextEdit{
                    id:memorySlotsValue
                    color:"#ffa8fc"
                    leftPadding: 4
                    anchors.verticalCenter: memSlotsLabel.verticalCenter
                    anchors.left:memSlotsLabel.right
                    anchors.right:parent.right
                    font.pixelSize:11
                    readOnly: true
                    overwriteMode: false
                    selectByMouse: true
                    horizontalAlignment: Text.AlignHCenter
                    text:""
                }
            }

            Rectangle{
                id: rawBackground
                color:"#585858"
                radius:6
                border.width: 1
                height: 32
                width: 352
                anchors{
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: 4
                }


                Text{
                    anchors{
                        left:parent.left
                        top:parent.top
                        leftMargin: 4
                    }
                    text:"RAW HEX [0:22]"
                    font.bold: true
                    color:"#a4a4a4"
                    font.pixelSize: 13
                }

                TextEdit{
                    id:raw
                    color:"white"
                    leftPadding: 4
                    rightPadding: 4
                    anchors.bottom:parent.bottom
                    anchors.left:parent.left
                    anchors.right:parent.right
                    font.pixelSize: 12
                    readOnly: true
                    overwriteMode: false
                    selectByMouse: true
                    horizontalAlignment: Text.Right
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    text:""
                }
            }
            Image{
                id: avatar
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -26
                fillMode: Image.PreserveAspectFit
                function setRight(){
                    anchors.left = parent.right;
                    anchors.right = undefined;
                    anchors.rightMargin = 0;
                    anchors.leftMargin = -22;
                    source = "qrc:/images/kraken_avatar.png";
                }

                function setLeft(){
                    anchors.left = undefined;
                    anchors.right = parent.left;
                    anchors.rightMargin = -22;
                    anchors.leftMargin = 0;
                    source = "qrc:/images/Droplet.png";
                }

                width:48
                height:width;
            }
        }
    }

    Component.onCompleted: {
        //KrakenZDriver.sendBitmap();
    }
}
