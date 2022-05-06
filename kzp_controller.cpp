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
static constexpr char EMPTY[] = "";



#ifdef Q_OS_LINUX
constexpr char OPEN_DENIED_STR[] = "Failed to open root USB device\n\nCheck if another process has control\n of the Kraken Z and UDEV rules are setup correctly";
#else
constexpr char OPEN_DENIED_STR[] = "Failed to open root USB device\n\nCheck if another process (NZXT CAM)\nis open and has control of the Kraken Z";
#endif


KZPController::KZPController(QApplication *parent)
    : QObject{parent}, mAppliedSettings{false}, mUxEngine{nullptr}, mState{ApplicationState::STARTING},
      mController{nullptr}, mKrakenAppController{nullptr}, mApplicationIcon{nullptr}, mSystemTray{nullptr},
      mPreviewWindow{nullptr}
{
    KrakenZDriverSelect::initializeDriverSelect(parent, KrakenZDriverSelect::SOFTWARE);
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




void KZPController::createPreviewWindow()
{
    if(mUxEngine && !mPreviewWindow) {
        // create the preview window component
        auto pw_component = new QQmlComponent(mUxEngine, QUrl(PREVIEW_WINDOW));
        if(pw_component && pw_component->isReady()) {
            createPreviewItem(pw_component);
        }else {
            connect(pw_component, &QQmlComponent::statusChanged, this, &KZPController::previewComponentReady);
        }
        // load it
    }
}

void KZPController::createPreviewItem(QQmlComponent* pw_component)
{
    if(!mPreviewWindow) {
        // load preview settings, use for create statement
        mPreviewWindow = qobject_cast<QQuickWindow*>(pw_component->create());
    }
}

void KZPController::previewComponentReady(QQmlComponent::Status status)
{
    auto pw_component{qobject_cast<QQmlComponent*>(sender())};
    if(!pw_component){
        return;
    }
    if(status == QQmlComponent::Ready) {
        createPreviewItem(pw_component);
    } else {
        qDebug() << "Failed to create preview window component\n" << pw_component->errorString();
        pw_component->deleteLater();
    }
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
    if(mKrakenAppController && mKrakenAppController->detachedPreview()){ // moving to detached
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
        connect(mKrakenAppController, &KrakenAppController::previewDetached, this, &KZPController::previewDetached);
        // setup the image provider
        success = true;
    }else {
        qDebug() << "Background Application Controller";
        QCoreApplication::exit(-1);
    }
    return success;
}

void KZPController::previewDetached(bool detached)
{
    // in foreground - if the main winodw closes,  and detached move to detached
    // in detached - if changes to not detached, move to foreground mode
    // in background - if detached becomes true, move to detached, else ignore.
    // in detached - if preview window closed, move to background
    switch(mState) {
        case ApplicationState::FOREGROUND:{
            if(!detached) {
                mPreviewWindow = nullptr;
            }
            break;
        }
        case ApplicationState::BACKGROUND: {
            if(detached){
                mState = ApplicationState::DETACHED;
                setMainWindow();
            }
            break;
        }
        case ApplicationState::DETACHED:{
            if(!detached){
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

bool KZPController::initializeMainWindow()
{
    bool created{mUxEngine == nullptr};
    if(created) {
        mUxEngine = new QQmlApplicationEngine(this);
        connect(mUxEngine, &QQmlApplicationEngine::objectCreated, this, &KZPController::connectToWindow);
    }
    return created;
}
void KZPController::setPreviewWindow(QObject* window)
{
    if(window == mPreviewWindow){
        return;
    }
    mPreviewWindow = qobject_cast<QQuickWindow*>(window);
    connect(mPreviewWindow, &QQuickItem::destroyed, this, [this](){
       mPreviewWindow = nullptr;
    });
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
    delete mUxEngine;
    mUxEngine = nullptr;
    delete mSystemTray;
    mSystemTray = nullptr;
}

KZPController::~KZPController()
{
    cleanUp();
}
