#include "kzp_controller.h"
#include "kraken_appcontroller.h"
#include <QQmlApplicationEngine>
#include <QApplication>
#include <QQmlContext>
#include "krakenimageprovider.h"
#include "systemtray.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <QFile>
#include <QPixmap>
#include <QQuickWindow>
#include <QTimer>
#include "settings.h"
#include "kzp_keys.h"

static constexpr char USER_WARNING[] = "qrc:/qml/UserWarning.qml";
static constexpr char KRAKEN_Z[] = "qrc:/qml/KrakenZ.qml";
static constexpr char CONFIGURE_WINDOW[] = "qrc:/qml/ConfigureWindow.qml";
static constexpr char PREVIEW_WINDOW[] = "qrc:/qml/PreviewWindow.qml";
static constexpr char PERMISSION_ERROR[] = "qrc:/qml/PermissionDenied.qml";
static constexpr char SETTINGS_ERROR[] = "qrc:/qml/SettingsError.qml";
static constexpr char MISSING_PROFILE[] = "qrc:/qml/MissingProfile.qml";



#ifdef Q_OS_LINUX
constexpr char OPEN_DENIED_STR[] = "Failed to open root USB device\n\nCheck if another process has control\n of the Kraken Z and UDEV rules are setup correctly";
#else
constexpr char OPEN_DENIED_STR[] = "Failed to open root USB device\n\nCheck if another process (NZXT CAM)\nis open and has control of the Kraken Z";
#endif


KZPController::KZPController(QApplication *parent)
    : QObject{parent}, mAppliedSettings{false}, mUxEngine{nullptr}, mState{ApplicationState::STARTING},
      mDriverSelect{parent, KrakenZDriverSelect::HARDWARE},  mController{nullptr},
      mKrakenAppController{nullptr}, mApplicationIcon{nullptr}, mSystemTray{nullptr}
{
    connect(parent, &QApplication::aboutToQuit, this, &KZPController::applicationQuiting);
    if(parent){
        QPixmap iconPixmap(":/images/Droplet.png");
        mApplicationIcon = iconPixmap;
        parent->setWindowIcon(mApplicationIcon);
    }
}

void KZPController::acceptUserAgreement()
{
    if(mState == ApplicationState::FIRST_TIME)
    {
        mState = ApplicationState::CONFIGURE_DRIVER;
        initializeBackend();
        releaseMainWindow();
    }
}

void KZPController::configured()
{
    if(mState == ApplicationState::CONFIGURE_DRIVER)
    {
        mState = ApplicationState::FOREGROUND;
        QTimer::singleShot(200, this, [this]{
            if(mKrakenAppController) {
                mKrakenAppController->loadImage("qrc:/images/Peyton.png");
            }
        });
        releaseMainWindow();
    }
}

void KZPController::selectSoftwareDriver()
{
    if(mState == ApplicationState::ERROR_PERMISSION || mState == ApplicationState::ERROR_DEVICE_NF) {
        mState = mStateBeforeLastError;
        if(mState >= FOREGROUND){
            if(mController) {
                mController->setJsonProfile(mProfile.value("krakenzdriver").toObject());
            }
            if(mKrakenAppController) {
                mKrakenAppController->setJsonProfile(mProfile.value("appcontroller").toObject());
            }
        }
    }
    setSoftwareController();
    releaseMainWindow();
}

void KZPController::applicationInitialize()
{
    setMainWindow();
}

void KZPController::applicationQuiting() // close up shop
{
    if(mController && mController->initialized() && mState >= ApplicationState::BACKGROUND) { // add setting for on exit
        mController->setNZXTMonitor();
    }
    if(mKrakenAppController) {
        mKrakenAppController->closeQmlApplications();
    }
    if(mState == ApplicationState::BACKGROUND || mState == ApplicationState::FOREGROUND) {
        writeSettingsFile();
    }
    cleanUp();
}


void KZPController::backgroundContainerReady()
{
    if(!mAppliedSettings){
       // mKrakenAppController->setJsonProfile(mProfile.value("appcontroller").toObject());
        mAppliedSettings = true;

    }
}

void KZPController::connectToWindow()
{
    auto objects = mUxEngine->rootObjects();
    if(objects.size()) {
        auto window{qobject_cast<QQuickWindow*>(objects.at(0))};
        if(window) {
            connect(window, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(moveToBackground()));
        }
    }

}


void KZPController::cleanUpWindow()
{
    if(mUxEngine) {
        mUxEngine->removeImageProvider("krakenz");
        mUxEngine->clearComponentCache();
        mUxEngine->collectGarbage();
        delete mUxEngine;
        mUxEngine = nullptr;
    }
    setMainWindow();
}

void KZPController::createSystemTray()
{
    mSystemTray = new SystemTray(this);
    connect(mSystemTray, &SystemTray::showMainWindow, this, &KZPController::showMainWindow);
    connect(mSystemTray, &SystemTray::profileSelected, this, &KZPController::setProfile);
    mSystemTray->setIcon(mApplicationIcon);
    mSystemTray->setVisible(true);
    QApplication::setQuitOnLastWindowClosed(false);
}

void KZPController::moveToBackground()
{
    if(mState <= ApplicationState::BACKGROUND) {
        QApplication::exit(0);
        return;
    }
    mState = ApplicationState::BACKGROUND;
    setMainWindow();
}

bool KZPController::createDeviceController()
{
    bool success{true};
    if(!mController) {
        mController = mDriverSelect.currentDriver();
        if(!mController){
            qDebug() << "Failed to create device Controller";
            QCoreApplication::exit(-1);
        }else {
            if(mController->found()){
                bool denied{false};
                if(!mController->initialize(denied) || denied){
                    mStateBeforeLastError = mState;
                    mState = ApplicationState::ERROR_PERMISSION;
                    QJsonObject error_obj;
                    error_obj.insert(TYPE_KEY, mState);
                    error_obj.insert(MSG_KEY, "Software Driver Available");
                    error_obj.insert(ERROR_KEY, OPEN_DENIED_STR);
                    mSettingsObject.insert("ControllerStatus", error_obj);
                    success = false;
                    mController = nullptr;
                }
            }else {
                success = false;
                mStateBeforeLastError = mState;
                mState = ApplicationState::ERROR_DEVICE_NF;
                QJsonObject error_obj;
                error_obj.insert(TYPE_KEY, mState);
                error_obj.insert(MSG_KEY, "Software Driver Available");
                mSettingsObject.insert("ControllerStatus", error_obj);
                mController = nullptr;
            }
        }
    }
    return success;
}

bool KZPController::setSoftwareController()
{
    auto previousController{mController};
    mController->disconnect();
    mDriverSelect.selectDriver(KrakenZDriverSelect::SOFTWARE);
    mController = mDriverSelect.currentDriver();
    if(mKrakenAppController){
        mKrakenAppController->setController(mController);
    }
    mSettingsObject.remove("ControllerStatus");
    return (previousController != mController);
}

void KZPController::writeSettingsFile()
{
//    QFile settingsFile(mSettingsDir + QDir::separator() + SETTINGS_FNAME);
//    if(settingsFile.open(QFile::WriteOnly)) {
//        QJsonObject profile;
//        profile.insert("krakenzdriver", mController->toJsonProfile());
//        profile.insert("appcontroller", mKrakenAppController->toJsonProfile());
//        if(mActiveProfile.size() == 0) {
//            // create default profile


//            writeCurrentSettings();
//        }else {
//            auto profiles = mSettingsObject.value("profiles").toArray();
//            auto profileCount = profiles.size();
//            for(int index{0}; index < profileCount; ++index){
//                auto profile = profiles.at(index).toObject();
//                auto name = profile.value("name").toString();
//                if(name.compare(mProfileName) == 0) { // found startup profile
//                    auto default_profile = defaultProfileObject();
//                    default_profile.insert("data", current_settings);
//                    default_profile.insert("name", mProfileName);
//                    profiles.replace(index,default_profile);
//                    mSettingsObject.insert("profiles", profiles);
//                    writeCurrentSettings();
//                }
//            }
//        }
  //  }
}

bool KZPController::createKrakenApplicationController()
{
    bool success{false};
    auto appPtr{ qobject_cast<QApplication*>(parent())};
    if(appPtr && !mKrakenAppController) {
        // Create the Background Application Controller
        mKrakenAppController =  new KrakenAppController(mController, appPtr);
        mKrakenAppController->setAlphaSize(8); // Configured for Kraken Device, could be adapted to other displays...
        mKrakenAppController->setBlueSize(8);
        mKrakenAppController->setDepthSize(32);
        mKrakenAppController->setGreenSize(8);
        mKrakenAppController->setRedSize(8);
        mKrakenAppController->setScreenSize(QSize(320,320));
        mKrakenAppController->setStencilSize(16);
        mKrakenAppController->setPrimaryScreen(appPtr->primaryScreen());
        connect(mKrakenAppController, &KrakenAppController::orientationChanged,
                         mController,  &KrakenZInterface::setScreenOrientation);
        connect(mKrakenAppController, &KrakenAppController::frameReady, this, &KZPController::processBackgroundFrame);
        // setup the image provider
        success = true;
    }else {
        qDebug() << "Background Application Controller";
        QCoreApplication::exit(-1);
    }
    return success;
}


void KZPController::processBackgroundFrame(QImage frame)
{
    if(mController){
        mController->setImage(frame,  mController->bucket() ^ 1);
    }
}

void KZPController::initializeBackend()
{
    if(!mController){
        createDeviceController();
    }
    if(!mKrakenAppController){
        if(createKrakenApplicationController()){
            connect(mKrakenAppController, &KrakenAppController::initialized, this, &KZPController::backgroundContainerReady);
            mKrakenAppController->initialize();
        }
    }
}

void KZPController::initializeMainWindow()
{
    if(!mUxEngine) {
        mUxEngine = new QQmlApplicationEngine(this);
        connect(mUxEngine, &QQmlApplicationEngine::objectCreated, this, &KZPController::connectToWindow);
    }
}

void KZPController::setMainWindow()
{
    QString qmlFile;
    switch(mState) {
        case ApplicationState::FIRST_TIME: {
            initializeMainWindow();
            mUxEngine->rootContext()->setContextProperty("KZP", this);
            qmlFile = USER_WARNING;
            break;
        }
        case ApplicationState::CONFIGURE_DRIVER: {
            initializeMainWindow();
            mUxEngine->rootContext()->setContextProperty("KZP", this);
            mUxEngine->rootContext()->setContextProperty("KrakenZDriver", mController);
            mUxEngine->rootContext()->setContextProperty("AppController", mKrakenAppController);
            qmlFile = CONFIGURE_WINDOW;
            break;
        }
        case ApplicationState::BACKGROUND: {
            releaseMainWindow();
            if(!mSystemTray){
                createSystemTray();
            }
            break;
        }
        case ApplicationState::FOREGROUND: {
            initializeMainWindow();
            qmlFile = KRAKEN_Z;
            auto previewProvider {new KrakenImageProvider()}; // uses preview
            mUxEngine->rootContext()->setContextProperty("KrakenZDriver", mController);
            mUxEngine->rootContext()->setContextProperty("AppController", mKrakenAppController);
            mUxEngine->addImageProvider("krakenz", previewProvider); // will be owned by the engine
            mUxEngine->rootContext()->setContextProperty("KrakenImageProvider", previewProvider);
            QObject::connect(mKrakenAppController, &KrakenAppController::frameReady, previewProvider, &KrakenImageProvider::imageChanged);
            mUxEngine->rootContext()->setContextProperty("ApplicationData", QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
            if(!mSystemTray){
                createSystemTray();
            }

            break;
        }
        case ApplicationState::ERROR_DEVICE_NF:
        case ApplicationState::ERROR_PERMISSION:
        {
            // offer software driver
            initializeMainWindow();
            qmlFile = PERMISSION_ERROR;
            mUxEngine->rootContext()->setContextProperty("KZP", this);
            mUxEngine->rootContext()->setContextProperty("ControllerStatus", mSettingsObject.value("ControllerStatus").toObject());
            break;
        }
        case ApplicationState::ERROR_PROFILE_NF: {
            initializeMainWindow();
            qmlFile = MISSING_PROFILE;
            mUxEngine->rootContext()->setContextProperty("ProfileName", mActiveProfile);
            break;
        }
        case ApplicationState::ERROR_PROFILES:
        case ApplicationState::ERROR_SETTINGS_NF:
        case ApplicationState::ERROR_PARSE_SETTINGS:
        {
            initializeMainWindow();
            qmlFile = SETTINGS_ERROR;
            mUxEngine->rootContext()->setContextProperty("SettingsStatus", mSettingsObject.value("SettingsStatus").toObject());
            break;
        }
        default: break;
    }
    if(mUxEngine && qmlFile.size()) {
        QUrl url(qmlFile);
        mUxEngine->load(url);
    }
}

void KZPController::releaseMainWindow()
{
    if(mUxEngine) {
        auto objects = mUxEngine->rootObjects();
        for(auto & object : objects ){
            object->deleteLater();
        }
        QTimer::singleShot(10, this, &KZPController::cleanUpWindow);
    }
}


void KZPController::showMainWindow()
{
    mState = ApplicationState::FOREGROUND;
    setMainWindow();
}


void KZPController::setProfile(QString name)
{
    qDebug() << "Profile set to " << name;
}

void KZPController::setSettingsConfiguration(QString directory, QString profile_name, bool userDirectory)
{
    int state{0};
    mActiveProfile = profile_name;
    auto settingsOutput = Settings::loadSettings(directory, mActiveProfile, state, userDirectory);
    if(state > ApplicationState::FOREGROUND) {
        mStateBeforeLastError = mState;
    }
    mState = ApplicationState(state);
    switch(mState) { // prepare system based on settings
        case ApplicationState::FIRST_TIME:
        case ApplicationState::CONFIGURE_DRIVER: {
            break;
        }
        case ApplicationState::BACKGROUND:
        case ApplicationState::FOREGROUND: {// correctly parsed, load user settings
            mSettingsObject = settingsOutput;
            mProfile = mSettingsObject.value("activeProfile").toObject();
            initializeBackend();
            // Apply the loaded properties
            if(mController) {
                mController->setJsonProfile(mProfile.value("krakenzdriver").toObject());
            }
            if(mKrakenAppController) {
                mKrakenAppController->setJsonProfile(mProfile.value("appcontroller").toObject());
            }
            break;
        }
        case ApplicationState::ERROR_DEVICE_NF:
        {
            // offer software driver
            break;
        }
        case ApplicationState::ERROR_PERMISSION:
        {
            // offer software driver
            break;
        }
        case ApplicationState::ERROR_SETTINGS_NF:
        case ApplicationState::ERROR_PROFILES:
        case ApplicationState::ERROR_PARSE_SETTINGS:
        {
            mSettingsObject.insert("SettingsStatus", settingsOutput);
            break;
        }
        default: break;
    }

}

void KZPController::cleanUp()
{
    delete mKrakenAppController;
    mKrakenAppController = nullptr;
    releaseMainWindow();
    mDriverSelect.releaseDrivers();
    delete mUxEngine;
    mUxEngine = nullptr;
    delete mSystemTray;
    mSystemTray = nullptr;
}

KZPController::~KZPController()
{
    cleanUp();
}
