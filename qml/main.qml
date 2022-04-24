import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    id:window
    width: 600
    height:720
    visible: false
    minimumHeight: 720
    minimumWidth:600
    maximumHeight:720
    maximumWidth: 600

    Timer{
        id: imageSetTimer
        property string imageSource: ""
        interval:100
        repeat: false
        running: false
        onTriggered: {
            AppController.loadImage(imageSource);
        }
    }
    onVisibleChanged: {
        KrakenImageProvider.setDisplayVisible(visible);
    }
    Rectangle { // Main content container
        id: container
        anchors.fill: parent
        color:"black"
        property string errorMesage:""
        Connections{
            target:KrakenZDriver
            enabled: container.state == ""
            function onError(error){
                container.errorMesage = error.MESSAGE;
                container.state = "error";
            }
            function onDeviceReady() {
                container.errorMesage = "";
                container.state = "configure"
            }
        }
        state:""

        states:[
            State {
                name:"" // default user Warning
                PropertyChanges{target:userWarningLoader; active: true}
                PropertyChanges{target:deviceConfigure; active: false}
                PropertyChanges{target:accessError; active: false}
                PropertyChanges{target:missingDevice; active: false}
                PropertyChanges{target:mainApplication; active: false}
            },
            State {
                name:"configure"
                PropertyChanges{target:userWarningLoader; active: false}
                PropertyChanges{target:deviceConfigure; active: true}
                PropertyChanges{target:accessError; active: false}
                PropertyChanges{target:missingDevice; active: false}
                PropertyChanges{target:mainApplication; active: false}
            },
            State {
                name: "error"
                PropertyChanges{target:userWarningLoader; active: false}
                PropertyChanges{target:deviceConfigure; active: false}
                PropertyChanges{target:accessError; active: true}
                PropertyChanges{target:missingDevice; active: false}
                PropertyChanges{target:mainApplication; active: false}
            },
            State {
                name:"missing"
                PropertyChanges{target:userWarningLoader; active: false}
                PropertyChanges{target:deviceConfigure; active: false}
                PropertyChanges{target:accessError; active: false}
                PropertyChanges{target:missingDevice; active: true}
                PropertyChanges{target:mainApplication; active: false}
            },
            State {
                name:"application"
                PropertyChanges{target:userWarningLoader; active: false}
                PropertyChanges{target:deviceConfigure; active: false}
                PropertyChanges{target:accessError; active: false}
                PropertyChanges{target:missingDevice; active: false}
                PropertyChanges{target:mainApplication; active: true}
            }
        ]
        Loader{
            id: mainApplication
            anchors.fill: parent
            active:false
            sourceComponent: KrakenZ{}
        }
        Loader{
            id:deviceConfigure
            active:false
            anchors.fill: parent
            sourceComponent: KrakenZConfigure {
                onConfigured:{
                    KrakenZDriver.setBuiltIn(1);
                    KrakenZDriver.setMonitorFPS();
                    imageSetTimer.imageSource = "qrc:/images/Peyton.png";
                    imageSetTimer.start();
                    container.state = "application"
                }
            }
        }
        Loader {
            id: accessError
            active: false
            anchors.fill:parent
            sourceComponent: PermissionDenied {

            }
            onLoaded: {
                item.errorMessage = container.errorMesage;
            }
        }
        Loader {
            id: missingDevice
            active: false
            anchors.fill:parent
            sourceComponent: MissingDevice {

            }
        }
        Loader{
            id:userWarningLoader
            active:true
            anchors.fill: parent
            sourceComponent:UserWarning{
                onAccepted:{
                    if(KrakenZDriver.found){
                        AppController.initialize();
                        KrakenZDriver.initialize();
                        SettingsManager.createDefaultSettings();
                    }else {
                        container.state = "missing";
                    }
                }
                Component.onCompleted: {
                    if(SettingsManager.acceptedAgreement) {                        
                        if(KrakenZDriver.found){
                            AppController.initialize();
                            KrakenZDriver.initialize();
                            container.state = "application"
                        }else {
                            container.state = "missing";
                            window.visible = true;
                        }
                    } else {
                        window.visible = true;
                    }
                }
            }
        }

    }
    Component.onCompleted: {
        SystemTray.setVisible();
    }
}
