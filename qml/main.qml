import QtQuick 2.15
import QtQuick.Window 2.15
import com.kzp.screens 1.0

Window {
    id:window
    width: 600
    height:720
    visible: false
    minimumHeight: 720
    minimumWidth:600
    maximumHeight:720
    maximumWidth: 600
    KZPController{
        id: appController
        container:container
    }

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
            function on(error){
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
                PropertyChanges{target:settingsError; active: false}
                PropertyChanges{target:missingDevice; active: false}
                PropertyChanges{target:mainApplication; active: false}
            },
            State {
                name:"configure"
                PropertyChanges{target:userWarningLoader; active: false}
                PropertyChanges{target:deviceConfigure; active: true}
                PropertyChanges{target:accessError; active: false}
                PropertyChanges{target:settingsError; active: false}
                PropertyChanges{target:missingDevice; active: false}
                PropertyChanges{target:mainApplication; active: false}
            },
            State {
                name: "error"
                PropertyChanges{target:userWarningLoader; active: false}
                PropertyChanges{target:deviceConfigure; active: false}
                PropertyChanges{target:accessError; active: true}
                PropertyChanges{target:window; visible: true}
                PropertyChanges{target:settingsError; active: false}
                PropertyChanges{target:missingDevice; active: false}
                PropertyChanges{target:mainApplication; active: false}
            },
            State {
                name: "settings_error"
                PropertyChanges{target:userWarningLoader; active: false}
                PropertyChanges{target:deviceConfigure; active: false}
                PropertyChanges{target:accessError; active: false}
                PropertyChanges{target:window; visible: true}
                PropertyChanges{target:settingsError; active: true}
                PropertyChanges{target:missingDevice; active: false}
                PropertyChanges{target:mainApplication; active: false}
            },
            State {
                name:"missing"
                PropertyChanges{target:userWarningLoader; active: false}
                PropertyChanges{target:deviceConfigure; active: false}
                PropertyChanges{target:accessError; active: false}
                PropertyChanges{target:window; visible: true}
                PropertyChanges{target:settingsError; active: false}
                PropertyChanges{target:missingDevice; active: true}
                PropertyChanges{target:mainApplication; active: false}
            },
            State {
                name:"application"
                PropertyChanges{target:userWarningLoader; active: false}
                PropertyChanges{target:deviceConfigure; active: false}
                PropertyChanges{target:accessError; active: false}
                PropertyChanges{target:settingsError; active: false}
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
            id: settingsError
            active: false
            anchors.fill:parent
            sourceComponent: SettingsError { }
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
                    var acceptedAgreement = SettingsManager.acceptedAgreement;
                    if(SettingsManager.errored) {
                        container.state = "settings_error";
                        return;
                    }

                    if(acceptedAgreement) {
                        if(KrakenZDriver.found){
                            SystemTray.setVisible();
                            AppController.initialize();
                            KrakenZDriver.initialize();
                            container.state = "application"
                        }else {
                            container.state = "missing";
                        }
                    }
                }
            }
        }

    }
}
