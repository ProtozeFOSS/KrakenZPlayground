#include "kzp_controller.h"
#include "kraken_appcontroller.h"
#include <QQmlApplicationEngine>
#include <QApplication>
#include <QQmlContext>
#include "preview_provider.h"
#include "system_tray.h"
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
    : QObject{parent}, mUxEngine{nullptr}, mState{ApplicationState::STARTING},
      mController{nullptr}, mKrakenAppController{nullptr}, mApplicationIcon{nullptr}, mSystemTray{nullptr},
      mPreviewWindow{nullptr}, mPreviewX{0}, mPreviewY{0}, mDetached{false}
{
    KrakenZDriverSelect::initializeDriverSelect(parent, KrakenZDriverSelect::HARDWARE);
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
    if(mState == ApplicationState::BACKGROUND || mState == ApplicationState::FOREGROUND || mState == ApplicationState::DETACHED) {
        writeSettingsFile();
    }
    cleanUp();
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
    if(mKrakenAppController && mDetached){ // moving to detached
        mState = ApplicationState::DETACHED;
    }else{
        mState = ApplicationState::BACKGROUND;
    }
    setMainWindow();
}

bool KZPController::createDeviceController()
{
    bool success{true};
    if(!mController) {
        auto driverSelect{KrakenZDriverSelect::GetInstance()};
        if(driverSelect){
            mController = driverSelect->currentDriver();
        }
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
                    mError.insert("ControllerStatus", error_obj);
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
                mError.insert("ControllerStatus", error_obj);
                mController = nullptr;
            }
        }
    }
    return success;
}

bool KZPController::setSoftwareController()
{
    auto previousController{mController};
    if(mController) {
        mController->disconnect();
    }
    auto driverSelect{KrakenZDriverSelect::GetInstance()};
    if(driverSelect) {
        driverSelect->selectDriver(KrakenZDriverSelect::SOFTWARE);
        mController = driverSelect->currentDriver();
    }
    if(mKrakenAppController){
        mKrakenAppController->setController(mController);
    }
    mError.remove("ControllerStatus");
    return (previousController != mController);
}

void KZPController::writeSettingsFile()
{
    auto filePath{Settings::getSettingsPath(mSettingsDir)};
    QFile settingsFile(filePath);
    QString profileName;
    int state;
    auto existingSettings = Settings::loadSettings(mSettingsDir,profileName,state,true);
    existingSettings.remove("activeProfile");
    if(settingsFile.open(QFile::WriteOnly)) {
        QJsonObject currentProfile = mProfile;
        currentProfile.insert("name", profileName);
        currentProfile.insert("state", mState);
        if(mController) {
            currentProfile.insert("krakenzdriver", mController->toJsonProfile());
        }
        if(mKrakenAppController) {
            currentProfile.insert("appcontroller", mKrakenAppController->toJsonProfile());
        }
        auto previewData{previewSettings()};
        currentProfile.insert("preview" , previewData);
        // find and update it in the array
        auto profiles = existingSettings.value("profiles").toArray();
        auto profileCount = profiles.size();
        for(int index{0}; index < profileCount; ++index){
            auto profile = profiles.at(index).toObject();
            auto name = profile.value("name").toString();
            if(name.compare(profileName) == 0) { // found startup profile
                profiles.replace(index,currentProfile);
                existingSettings.remove("profiles");
                existingSettings.insert("profiles", profiles);
            }
        }
        existingSettings.insert("startProfile", profileName);
        QJsonDocument doc;
        doc.setObject(existingSettings);
        settingsFile.write(doc.toJson());
    }
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

void KZPController::detachPreview(bool detach)
{
    if(mDetached != detach) {
        mDetached = detach;
        switch(mState) {
            case ApplicationState::FOREGROUND:{
                if(!mDetached) {
                    mPreviewWindow = nullptr;
                }
                break;
            }
            case ApplicationState::BACKGROUND: {
                if(mDetached){
                    mState = ApplicationState::DETACHED;
                    setMainWindow();
                }
                break;
            }
            case ApplicationState::DETACHED:{
                if(!mDetached){
                    mState = ApplicationState::BACKGROUND;
                    setMainWindow();
                }
            }
            default:;
        }
        emit previewDetached(detach);
    }
}


QJsonObject KZPController::previewSettings()
{
    QJsonObject preview;
    preview.insert("x", mPreviewX);
    preview.insert("y", mPreviewY);
    preview.insert("detached", mDetached);
    preview.insert("locked", mLocked);
    return preview;
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
            mKrakenAppController->initialize();
        }
    }
}

bool KZPController::initializeMainWindow()
{
    bool created{mUxEngine == nullptr};
    if(created) {
        mUxEngine = new QQmlApplicationEngine(this);
        connect(mUxEngine, &QQmlApplicationEngine::objectCreated, this, &KZPController::connectToWindow);
    }
    return created;
}

void KZPController::lockMovement(bool lock)
{
    if(lock != mLocked) {
        mLocked = lock;
        emit movementLocked(lock);
    }
}

void KZPController::loadProfile()
{
    if(!mProfile.isEmpty() && mProfile.contains("preview")){
        auto previewSettings = mProfile.value("preview").toObject();
            auto lValue = previewSettings.value("locked");
            if(!lValue.isNull()) {
                mLocked = lValue.toBool();
            }
            auto dValue = previewSettings.value("detached");
            if(!dValue.isNull()) {
                mDetached = dValue.toBool();
            }
            auto xValue = previewSettings.value("x");
            if(!xValue.isNull()) {
                mPreviewX = xValue.toDouble();
            }
            auto yValue = previewSettings.value("y");
            if(!yValue.isNull()) {
                mPreviewY = yValue.toDouble();
            }
        }
}

void KZPController::recordPreviewLocation(qreal x, qreal y)
{
    mPreviewX = x;
    mPreviewY = y;
}

void KZPController::setPreviewWindow(QObject* window)
{
    if(window == mPreviewWindow){
        return;
    }
    mPreviewWindow = qobject_cast<QQuickWindow*>(window);
    if(mPreviewWindow) {
        mPreviewWindow->setProperty("x", mPreviewX);
        mPreviewWindow->setProperty("y", mPreviewY);
        connect(mPreviewWindow, &QQuickItem::destroyed, this, [this](){
           mPreviewWindow = nullptr;
        });
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
            releaseMainWindow();
            if(initializeMainWindow()){
                qmlFile = KRAKEN_Z;
                setUxObjects();
                if(!mSystemTray){
                    createSystemTray();
                }
            }
            break;
        }
        case ApplicationState::DETACHED:{
            releaseMainWindow();
            if(initializeMainWindow()){
                qmlFile = PREVIEW_WINDOW;
                setUxObjects();
                if(!mSystemTray){
                    createSystemTray();
                }
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
            mUxEngine->rootContext()->setContextProperty("ControllerStatus", mError.value("ControllerStatus").toObject());
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
            mUxEngine->rootContext()->setContextProperty("SettingsStatus", mError.value("SettingsStatus").toObject());
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

void KZPController::setUxObjects()
{
    if(!mUxEngine->imageProvider("krakenz")){
        auto previewProvider {new ProxyImageProvider()}; // uses preview
        mUxEngine->rootContext()->setContextProperty("KZP", this);
        mUxEngine->rootContext()->setContextProperty("KrakenZDriver", mController);
        mUxEngine->rootContext()->setContextProperty("AppController", mKrakenAppController);
        mUxEngine->addImageProvider("krakenz", previewProvider); // will be owned by the engine
        mUxEngine->rootContext()->setContextProperty("KrakenImageProvider", previewProvider);
        QObject::connect(mKrakenAppController, &KrakenAppController::frameReady, previewProvider, &ProxyImageProvider::imageChanged);
        mUxEngine->rootContext()->setContextProperty("ApplicationData", QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
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
    mSettingsDir = directory;
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
        case ApplicationState::FOREGROUND:
        case ApplicationState::DETACHED: {// correctly parsed, load user settings
            mProfile = settingsOutput.value("activeProfile").toObject();
            settingsOutput.remove("activeProfile");
            initializeBackend();
            // Apply the loaded properties
            if(mController) {
                mController->setJsonProfile(mProfile.value("krakenzdriver").toObject());
            }
            if(mKrakenAppController) {
                mKrakenAppController->setJsonProfile(mProfile.value("appcontroller").toObject());
            }
            loadProfile();
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
            mError.insert("SettingsStatus", settingsOutput);
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
    delete mUxEngine;
    mUxEngine = nullptr;
    delete mSystemTray;
    mSystemTray = nullptr;
}

KZPController::~KZPController()
{
    cleanUp();
}
