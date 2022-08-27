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
#include "settings.h"
#include "kzp_keys.h"

static constexpr char USER_WARNING[] = "qrc:/qml/UserWarning.qml";
static constexpr char KRAKEN_Z[] = "qrc:/qml/KrakenZ.qml";
static constexpr char CONFIGURE_WINDOW[] = "qrc:/qml/ConfigureWindow.qml";
static constexpr char PREVIEW_WINDOW[] = "qrc:/qml/PreviewWindow.qml";
static constexpr char PERMISSION_ERROR[] = "qrc:/qml/PermissionDenied.qml";
static constexpr char SETTINGS_ERROR[] = "qrc:/qml/SettingsError.qml";
static constexpr char MISSING_PROFILE[] = "qrc:/qml/MissingProfile.qml";

SystemMonitor* KZPController::mSystemMonitor = nullptr;

#ifdef Q_OS_LINUX
constexpr char OPEN_DENIED_STR[] = "Failed to open root USB device\n\nCheck if another process has control\n of the Kraken Z and UDEV rules are setup correctly";
#else
constexpr char OPEN_DENIED_STR[] = "Failed to open root USB device\n\nCheck if another process (NZXT CAM)\nis open and has control of the Kraken Z";
#endif


KZPController::KZPController(QApplication *parent)
    : QObject{parent}, mUxEngine{nullptr}, mState{ApplicationState::STARTING},
      mController{nullptr}, mKrakenAppController{nullptr}, mApplicationIcon{nullptr}, mSystemTray{nullptr},
      mPreviewWindow{nullptr}, mPreview{this}, mModuleManager{this}
{
    connect(parent, &QApplication::aboutToQuit, this, &KZPController::applicationQuiting);
    if(parent){
        mRootDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QPixmap iconPixmap(":/images/Droplet.png");
        mApplicationIcon = iconPixmap;
        parent->setWindowIcon(mApplicationIcon);
    }
    connect(&mPreview, &PreviewWindow::detachChanged, this, &KZPController::detachPreview);

}

void KZPController::acceptUserAgreement()
{
    if(mState == ApplicationState::FIRST_TIME)
    {
        if(initializeBackend()) {
            mState = ApplicationState::CONFIGURE_DRIVER;
        }
        releaseMainWindow();
    }
}

void KZPController::configured()
{
    if(mState == ApplicationState::CONFIGURE_DRIVER)
    {
        mState = ApplicationState::FOREGROUND;
        mSystemMonitor = new SystemMonitor(this);
        QTimer::singleShot(400, this, [this]{
            if(mController && mKrakenAppController) {
                mController->setBuiltIn(0);
                mKrakenAppController->loadImage("qrc:/images/Peyton.png");
            }
        });
    }
    releaseMainWindow();
}


void KZPController::selectSoftwareDriver()
{
    setSoftwareController();
    if(mState == ApplicationState::ERROR_PERMISSION || mState == ApplicationState::ERROR_DEVICE_NF || mState == ApplicationState::STARTING) {
        if(mStateBeforeLastError == FIRST_TIME) {
            mState = ApplicationState::CONFIGURE_DRIVER;
        }else {
            mState = mStateBeforeLastError;
        }
        if(mState >= CONFIGURE_DRIVER){
            if(!mKrakenAppController){
                if(createKrakenApplicationController()){
                    mKrakenAppController->initialize();
                    mKrakenAppController->setJsonProfile(mProfile.value("appcontroller").toObject());
                }
            }
        }
    }
    releaseMainWindow();
}

void KZPController::applicationInitialize()
{
    setMainWindow();
    connectModuleManager();
    if(mSettings.autoUpdate){
        // mModuleManager.checkUpdates();
        // run background update check
        // mUpdateCheck = new UpdateCheck();
        // connect to updateCheck()
        // mUpdateCheck->start();
    }
    // mModuleCheck = new InstalledModuleCache()
    if(mSettings.autoUpdateModules) {
        // mModuleManager.checkModuleUpdates();
        // run
        // mModuleCheck = new CheckInstalledModules();
        // connect to moduleCheck)
        // mModuleCheck->start();
    }
}

void KZPController::applicationQuiting() // close up shop
{
    if(mUxEngine || mController || mKrakenAppController) {
        if(mSystemMonitor){
            mSystemMonitor->aboutToExit();
        }
        if(mController) {
            mController->setNZXTMonitor();
        }
        if(mKrakenAppController) {
            mKrakenAppController->closeQmlApplications();
        }
        if(mState == ApplicationState::BACKGROUND || mState == ApplicationState::FOREGROUND || mState == ApplicationState::DETACHED) {
            writeSettingsFile();
        }
        QTimer::singleShot(100,this,[](){
            //cleanUp();
            QCoreApplication::quit();
        });
    }
}


void KZPController::connectModuleManager()
{
    connect(&mModuleManager, &ModuleManager::installedModulesCheck, this, &KZPController::receivedInstalledManifests);
    connect(&mModuleManager, &ModuleManager::moduleManifests, this, &KZPController::receivedModuleManifests);
    connect(&mModuleManager, &ModuleManager::taskStarted, this, [](QString file, KZPCoroutines::TaskType type) {
       qDebug() << "Started task type:" << type << "on" <<  file;
    });
    connect(&mModuleManager, &ModuleManager::taskFinished, this, [](QString file, KZPCoroutines::TaskType type) {
        qDebug() << "Finished task type:" << type << "on" <<  file;
     });
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
    connect(mSystemTray, &SystemTray::close, this, &KZPController::applicationQuiting);
    mSystemTray->setIcon(mApplicationIcon);
    mSystemTray->setVisible(true);
    QApplication::setQuitOnLastWindowClosed(false);
}


void KZPController::moveToBackground()
{    
    mPreview.showSettings(false);
    if(mState <= ApplicationState::BACKGROUND) {
        QApplication::quit();
        return;
    }
    if(mKrakenAppController && mPreview.mDetached){ // moving to detached
        mState = ApplicationState::DETACHED;
    }else{
        mState = ApplicationState::BACKGROUND;
    }
    setMainWindow();
}

bool KZPController::createDeviceController()
{
    bool success{false};
    auto app{qobject_cast<QApplication*>(parent())};
    if(!mController) {
        auto driverSelect{KrakenZDriverSelect::GetInstance()};
        if(!driverSelect){
             KrakenZDriverSelect::initializeDriverSelect(app, KrakenZDriverSelect::HARDWARE);
             driverSelect = KrakenZDriverSelect::GetInstance();
        }
        if(driverSelect){
            mController = driverSelect->currentDriver();
        }else {
            qDebug() << "Failed to create device Controller";
            app->quit();
            return false;
        }
        if(mController){
            if(mController->found()){
                bool denied{false};
                if(!mController->initialize(denied)){
                    mStateBeforeLastError = mState;
                    mState = ApplicationState::ERROR_PERMISSION;
                    QJsonObject error_obj;
                    error_obj.insert(SharedKeys::Type, mState);
                    error_obj.insert(SharedKeys::Message, "Software Driver Available");
                    error_obj.insert(SharedKeys::ErrorString, OPEN_DENIED_STR);
                    mError.insert("ControllerStatus", error_obj);
                    mController = nullptr;
                } else {
                    success = true;
                }
            }else {
                mStateBeforeLastError = mState;
                mState = ApplicationState::ERROR_DEVICE_NF;
                QJsonObject error_obj;
                error_obj.insert(SharedKeys::Type, mState);
                error_obj.insert(SharedKeys::Message, "Software Driver Available");
                mError.insert("ControllerStatus", error_obj);
                mController = nullptr;
            }
        }
    }
    return success;
}

SystemMonitor* KZPController::getSystemMonitor()
{
    return mSystemMonitor;
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
    if(mController) {
        bool denied;
        mController->initialize(denied);
        mController->setJsonProfile(mProfile.value("krakenzdriver").toObject());
        if(mKrakenAppController){
            mKrakenAppController->setController(mController);
        }
    }
    mError.remove("ControllerStatus");
    return (previousController != mController);
}

void KZPController::writeSettingsFile()
{
    auto filePath{Settings::getSettingsPath(mSettingsDir)};
    QFile settingsFile(filePath);
    QString profileName{QStringLiteral("Default")};
    int state;
    auto existingSettings = Settings::loadSettings(mSettingsDir,profileName,state,true);
    if(existingSettings.contains(SharedKeys::ErrorString)) {
        existingSettings = QJsonObject{};
    }
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
        auto previewData{mPreview.profileData()};
        currentProfile.insert("preview" , previewData);
        if(mSystemMonitor) {
            auto sensorData{mSystemMonitor->jsonSettings()};
            auto sensor_path = getSensorPath();
            if(!sensorData.isEmpty() && sensor_path.size() > 0) {
                currentProfile.insert(sensor_path, sensorData);
            }
        }
        // find and update it in the array
        auto profiles = existingSettings.value(SharedKeys::Profiles).toArray();

        auto profileCount = profiles.size();
        if(profileCount == 0) {
            profiles.append(currentProfile);
        } else {
            for(int index{0}; index < profileCount; ++index){
                auto profile = profiles.at(index).toObject();
                auto name = profile.value("name").toString();
                if(name.compare(profileName) == 0) { // found startup profile
                    profiles.replace(index,currentProfile);
                    existingSettings.remove(SharedKeys::Profiles);
                    existingSettings.insert(SharedKeys::Profiles, profiles);
                }
            }
        }
        existingSettings.insert(SharedKeys::Profiles, profiles);
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
        mKrakenAppController =  new KrakenAppController(&mPreview, mController, mSystemMonitor, appPtr);
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
        QCoreApplication::quit();
    }
    return success;
}

void KZPController::detachPreview(bool detach)
{
    switch(mState) {
    case ApplicationState::FOREGROUND:{
        if(!detach) {
            mPreviewWindow = nullptr;
        }
        break;
    }
    case ApplicationState::BACKGROUND: {
        if(detach){
            mState = ApplicationState::DETACHED;
            setMainWindow();
        }
        break;
    }
    case ApplicationState::DETACHED:{
        if(!detach){
            mState = ApplicationState::BACKGROUND;
            setMainWindow();
        }
    }
    default:;
    }

}


void KZPController::processBackgroundFrame(QImage frame)
{
    if(mController){
        mController->setImage(frame,  mController->bucket() ^ 1);
    }
}

bool KZPController::initializeBackend()
{
    if(!mController){
        if(createDeviceController()) {
            if(!mKrakenAppController){
                if(createKrakenApplicationController()){
                    mKrakenAppController->initialize();
                    return true;
                }
            }
        }
    }
    return false;
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


void KZPController::loadProfile()
{
    if(mProfile.isEmpty()){
        return;
    }
    if(mProfile.contains("preview")){
        mPreview.setProfileData(mProfile.value("preview").toObject());
    }
    if(mSystemMonitor) {
        mSystemMonitor->initializeSensors(mProfile.value(getSensorPath()).toObject());
    }

}

void KZPController::loadManifest(QJsonObject manifest_obj)
{
    mPreview.setSettingsPath(QString());
    if(!manifest_obj.isEmpty()) {
        auto name{manifest_obj.value(SharedKeys::Name).toString()};
        if(name.size() == 0) {
            return;
        }
        auto settingsPath{manifest_obj.value(SharedKeys::Settings).toString()};
        if(settingsPath.size() > 0 ) {
            QDir dir;
            if(!dir.isAbsolutePath(settingsPath)) {
                settingsPath = mRootDir + name + "/" + settingsPath;
            }
        }
        mPreview.setSettingsPath(settingsPath);
        auto entry{manifest_obj.value(SharedKeys::Entry).toString()};
        if(entry.size() > 0) {
            mKrakenAppController->loadQmlFile(entry);
        }
    }
}


void KZPController::loadManifest(QString path)
{
    QFile f(path);
    if(f.open(QFile::ReadOnly)) {
        auto doc{ QJsonDocument::fromJson(f.readAll())};
        auto manifest = doc.object();
        loadManifest(manifest);
    }
}

void KZPController::setPreviewWindow(QObject* window)
{
    if(window == mPreviewWindow){
        return;
    }
    mPreviewWindow = qobject_cast<QQuickWindow*>(window);
    if(mPreviewWindow) {
        mPreviewWindow->setProperty("x", mPreview.mX);
        mPreviewWindow->setProperty("y", mPreview.mY);
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
            mUxEngine->rootContext()->setContextProperty("DeviceConnection", mController);
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

void KZPController::receivedInstalledManifests(QVector<QJsonObject> local_manifests)
{

}

void KZPController::receivedModuleManifests(QVector<ObjectReply> manifests)
{
    qDebug() << "Received " << manifests.size() << " Manifests ";
    for(const auto & manifest: qAsConst(manifests)) {
        qDebug() << manifest.second;
    }
}

void KZPController::releaseMainWindow()
{    
    mPreview.showSettings(false);
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
    mUxEngine->rootContext()->setContextProperty("KZP", this);
    mUxEngine->rootContext()->setContextProperty("Modules", &mModuleManager);
    if(mKrakenAppController && mSystemMonitor) {
        mUxEngine->rootContext()->setContextProperty("AppController", mKrakenAppController);
        mUxEngine->rootContext()->setContextProperty("SystemMonitor", mSystemMonitor);
        mKrakenAppController->setSystemMonitor(mSystemMonitor);
    }
    mUxEngine->rootContext()->setContextProperty("Preview", &mPreview);
    mUxEngine->rootContext()->setContextProperty("DeviceConnection", mController);
    mUxEngine->rootContext()->setContextProperty("ApplicationData", QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    if(!mUxEngine->imageProvider("krakenz")){
        auto previewProvider {new ProxyImageProvider()}; // uses preview
        mUxEngine->addImageProvider("krakenz", previewProvider); // will be owned by the engine
        mUxEngine->rootContext()->setContextProperty("KrakenImageProvider", previewProvider);
        QObject::connect(mKrakenAppController, &KrakenAppController::frameReady, previewProvider, &ProxyImageProvider::imageChanged);
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
    mModuleManager.setRootPath(mRootDir);
    switch(mState) { // prepare system based on settings
        case ApplicationState::FIRST_TIME:
        case ApplicationState::CONFIGURE_DRIVER: {
            break;
        }

        case ApplicationState::BACKGROUND:
        case ApplicationState::FOREGROUND:
        case ApplicationState::DETACHED: {// correctly parsed, load user settings
            mProfile = settingsOutput.value("activeProfile").toObject();
            mSystemMonitor = new SystemMonitor(this);
            auto kraken_settings = mProfile.value("krakenzdriver").toObject();
            auto software = kraken_settings.value(SharedKeys::Software).toBool(false);
            settingsOutput.remove("activeProfile");
            initializeBackend();
            // Apply the loaded properties
            if(mController) {
                mController->setJsonProfile(kraken_settings);
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
    releaseMainWindow();
    delete mUxEngine;
    mUxEngine = nullptr;
    delete mKrakenAppController;
    mKrakenAppController = nullptr;
    mController = nullptr;
    delete mSystemTray;
    mSystemTray = nullptr;
    delete mSystemMonitor;
    mSystemMonitor = nullptr;
    auto driverSelect{KrakenZDriverSelect::GetInstance()};
    driverSelect->releaseDrivers();
}

KZPController::~KZPController()
{
    cleanUp();
}




