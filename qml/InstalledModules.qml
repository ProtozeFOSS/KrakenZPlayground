import QtQuick 2.15

Rectangle{
    id: background
    anchors.fill: parent
    color:"#c0c0c0"
    radius:4
    clip:true
    MouseArea{
        anchors.fill: parent
        preventStealing: true
        onClicked: {

        }
    }
    property var moduleList:[]

    signal moduleSelected(module: var);
    signal canceled();
    signal openFromFile();
    Connections{
        target:Modules
        function onModuleFound(module) {
            var moduleText = "Found: " + module.name;
            if(statusText.text == "") {
                statusText.text = moduleText;
                background.moduleList.push(module);
                //moduleView.model = background.moduleList;
            } else {

                statusWindow.messages.push(moduleText);
                if(!textTimer.running) {
                    textTimer.start()
                }
            }
        }
        function onFinishedCachingLocal(module_list) {
            var newText = "Modules Cached. Found " + module_list.length +  " Modules";
            if(statusText.text == "") {
                statusText.text = newText;
            } else {
                statusWindow.messages.push(newText);
                if(!textTimer.running) {
                    textTimer.start()
                }
            }
            background.moduleList = []
            for(var i =0; i < module_list.length; ++i) {
                background.moduleList.push(module_list[i]);
            }
            moduleView.model = background.moduleList;
        }
    }

    Rectangle{
        height:24
        radius:6
        width:140
        anchors{
            top:parent.top
            horizontalCenter: parent.horizontalCenter
        }
        Text{
            anchors.fill: parent
            font.pixelSize: 16
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text:"Installed Modules"
        }
    }
    GridView{
        id:moduleView
        anchors{
            top:parent.top
            left:parent.left
            right:parent.right
            topMargin:42
        }
        height:parent.height - (contentTray.height + 32)
        clip:true
        anchors.margins: 4
        anchors.leftMargin: 42
        delegate:ModuleDelegate{
            id:moduleDelegate
            width: this.moduleLabel.paintedWidth <= 86 ? 86: this.moduleLabel.paintedWidth
            height:94;
            color: GridView.isCurrentItem ? "#1a3d58":"transparent"
            MouseArea{
                anchors.fill: parent
                onClicked:{
                    contentTray.contentObject = background.moduleList[index];
                    openTray.stop();
                    toggleTray.start();
                    moduleView.currentIndex = index;
                }
            }
        }
        highlightFollowsCurrentItem: true
        cellHeight:94
        cellWidth:82
        model:background.moduleList

        onCountChanged: {
            if(count == 0) {
                openTray.stop();
                toggleTray.stop();
                closeTray.start();
            }
        }
    }


    ContentTray{
        id: contentTray
        anchors.bottomMargin: -2
        anchors.bottom:statusWindow.top
        anchors.horizontalCenter: parent.horizontalCenter
        height:0
        width:parent.width* .98
        onDeleteContent: {
            let index = moduleView.currentIndex;
            let item = background.moduleList[index];
            console.log("Removed item: " + item.name);
            if(background.moduleList.length > 1) {
                background.moduleList.slice(index, 1);
                if(index > 0) {
                    moduleView.currentIndex = index-1;
                    contentTray.contentObject = background.moduleList[index-1];
                } else {
                    moduleView.currentIndex = 0;
                    contentTray.contentObject = background.moduleList[index];
                }
                openTray.stop();
                toggleTray.start();
            }else {
                console.log("Removing last");
                background.moduleList = []
                contentTray.contentObject = null;
            }
        }

    }

    SpringAnimation {
        id:openTray
        target:contentTray
        property:"height"
        running:false;
        alwaysRunToEnd:false
        duration:320
        spring:2
        damping:.1
        from:0
        to:204
    }
    NumberAnimation{
        id:closeTray
        target:contentTray
        property:"height"
        running:false;
        alwaysRunToEnd:true
        duration:240
        from:204
        to:0
    }
    NumberAnimation{
        id:toggleTray
        target:contentTray
        property:"height"
        running:false;
        alwaysRunToEnd:true
        duration:240
        to:0
        onFinished:{
            openTray.start();
        }
    }
    Rectangle{
        id:statusWindow
        property var messages: []
        border.width: 2
        color:"lightsteelblue"
        width:parent.width + 2
        height:28

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        Text{
            id:statusText
            leftPadding:6
            anchors.fill: parent
            font.pixelSize:16
            text:""
            verticalAlignment: Text.AlignVCenter
        }
        Timer{
            id: textTimer
            running:false
            interval: 125
            repeat: true
            onTriggered: {
                if(statusWindow.messages.length == 0) {
                    textTimer.stop();
                }else {
                    statusText.text = statusWindow.messages.shift();
                }
            }
        }
    }



    Rectangle{
        id:settingsLabel
        color: "transparent"
        radius:4
        width:84
        opacity:0
        anchors{
            right:settingsContainer.left
            rightMargin:14
            verticalCenter:settingsContainer.verticalCenter
            verticalCenterOffset: -2
        }
        Text{
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.fill: parent
            rightPadding:6
            font.bold: true
            font.pixelSize: 16
            font.letterSpacing: 2
            color:"black"
            text:"CONFIGURE"
            style:Text.Sunken
        }
    }
    ParallelAnimation{
        id:openSettingsLabel
        NumberAnimation{
            target:settingsLabel
            properties: "opacity"
            duration:180
            from:0
            to: 1.0
        }
        NumberAnimation{
            target:settingsIcon
            properties: "rotation"
            duration:200
            from:0
            to:360
        }
    }
    ParallelAnimation{
        id:closeSettingsLabel
        running:false
        alwaysRunToEnd: true
        NumberAnimation{
            target:settingsLabel
            properties: "opacity"
            duration:180
            from:1.0
            to: 0
        }
        NumberAnimation{
            target:settingsIcon
            properties: "rotation"
            duration:200
            to:0
            from:360
        }
    }
    Rectangle{
        id:settingsContainer
        color: "transparent"
        height:40
        width:40
        radius:12
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 2

        Rectangle{
            id:settingsIcon
            color: "#1054ca"
            anchors.fill: parent
            anchors.margins: 6
            radius:32
            Image{
                height:36
                width:36
                anchors.centerIn: parent
                antialiasing: true
                smooth: true
                source:"qrc:/images/settings.svg"
            }
        }

        MouseArea{
            anchors.fill: parent
            onClicked:{
                background.canceled();
                Modules.toggleModuleManager(true);
            }
            hoverEnabled:true
            onEntered:{
                closeSettingsLabel.stop();
                openSettingsLabel.start();
            }
            onExited: {
                openSettingsLabel.stop();
                closeSettingsLabel.start();
            }
        }
    }

    Rectangle{
        id:folderLabel
        color: "transparent"
        radius:4
        width:50
        opacity:0
        anchors{
            left:folderButton.right
            leftMargin:10
            verticalCenter:folderButton.verticalCenter
            verticalCenterOffset: -2
        }
        Text{
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
            anchors.fill: parent
            leftPadding:4
            font.bold: true
            font.pixelSize: 16
            font.letterSpacing: 2
            color:"black"
            text:"FILE"
            style:Text.Sunken
        }
    }
    NumberAnimation{
        id:openFolderLabel
        target:folderLabel
        properties: "opacity"
        duration:180
        from:0
        to: 1.0
    }
    NumberAnimation{
        id:closeFolderLabel
        target:folderLabel
        properties: "opacity"
        alwaysRunToEnd: true
        duration:180
        from:1.0
        to: 0
    }

    Rectangle{
        id:folderButton
        color: "transparent"
        height:40
        width:40
        radius:12
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 2

        Rectangle{
            id:folderIcon
            color: "#1054ca"
            anchors.fill: parent
            anchors.margins: 6
            radius:32
            Image{
                height:36
                width:36
                anchors.centerIn: parent
                antialiasing: true
                smooth: true
                source:"qrc:/images/folder.png"
            }
        }

        MouseArea{
            anchors.fill: parent
            onClicked:{
                background.openFromFile();
            }
            hoverEnabled:true
            onEntered:{
                closeFolderLabel.stop();
                openFolderLabel.start();
            }
            onExited: {
                openFolderLabel.stop();
                closeFolderLabel.start();
            }
        }
    }



    Rectangle{
        id:selectButton
        radius:12
        border.width:2
        border.color: "#2e2e2e"
        width:120
        height:42
        color: "#94cc94"
        anchors{
            right:cancelButton.left
            verticalCenter:cancelButton.verticalCenter
            margins: 8
        }
        Text{
            anchors.fill: parent
            text:"Set"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        MouseArea{
            anchors.fill: parent
            onClicked:{
                if(moduleView.currentItem) {
                    background.moduleSelected(background.moduleList[moduleView.currentIndex]);
                }
            }
        }
    }
    Rectangle{
        id:cancelButton
        width:120
        height:42
        color: "#ff6464"        
        radius:12
        border.width:2
        border.color: "#2e2e2e"
        anchors{
            right:parent.right
            bottom:parent.bottom
            margins: 4
        }
        Text{
            anchors.fill: parent
            text:"Cancel"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        MouseArea{
            anchors.fill: parent
            onClicked:{
                background.canceled();
            }
        }
    }
}
