#ifndef KZPCONTROLLER_H
#define KZPCONTROLLER_H

#include <QObject>
#include <QJsonObject>
#include "krakenz_driver.h"
#include <QIcon>
#include <QVector>
#include "modules.h"
#include "system_monitor.h"
#include <QTimer>

using namespace Modules;
class QApplication;
class QQuickItem;
class QQuickWindow;
class QQmlApplicationEngine;
class QQmlComponent;
class KrakenAppController;
class KrakenImageProvider;
class SystemTray;
enum class ApplicationState;

constexpr char APP_VERSION[] = "v1.2RC2";


struct AppSettings{
    bool autoUpdate{false};
    bool autoUpdateModules{false};
    // main window
    int  windowX{0};
    int  windowY{0};
};


// KZPController takes the UX logic from main.qml and the backend logic from main.cpp and puts them into a class.
// The goal, this object is on the stack in main and controlls the logic on the main thread
// This should improve the maintainability and readability of the main control logic.
// In a single C++ class the application state drives the UX instead of the UX controlling the
// application state. This flips the anti-pattern paradigm of main.qml exploding with lots of qml state logic (great for prototyping)

// Goal of KZP is to be minimal at all times until requested to be more. This means the resting state is in the system tray (background).
// When the user requests to do more, the system tray can notify KZPController of profile changes and the such without needing a window.
// If the user wants to adjust the profile, the main window will be created and shown.
// when moving back to background mode, KZP will clean up to maintain a clean footprint.


#include "preview_window.h"

class KZPController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ApplicationState state READ state NOTIFY stateChanged MEMBER mState)
    Q_PROPERTY(QString activeProfile READ activeProfile NOTIFY profileChanged MEMBER mActiveProfile)
    Q_PROPERTY(QString version READ applicationVersion CONSTANT)

public:
    enum ApplicationState{
      STARTING = -1,  // before settings is loaded
      FIRST_TIME = 0, // settings didnt exist, exit is close
      CONFIGURE_DRIVER = 1, // Made it to configuration close now is like accept and exit
      BACKGROUND = 2, // If everything went well, we made it here, enable systray
      FOREGROUND = 3, // Showing settings and preview (main window)
      DETACHED = 4, // Showing preview detached on desktop
      ERROR_PERMISSION = 5,
      ERROR_DEVICE_NF = 6, // show option for software driver
      ERROR_SETTINGS_NF = 7, // profile specified doesn't exist
      ERROR_PROFILE_NF = 8, // profile specified doesn't exist
      ERROR_PROFILES = 9, // profile specified doesn't exist
      ERROR_PARSE_SETTINGS = 10 // check json in file
    };
    Q_ENUM(ApplicationState)
    explicit KZPController(QApplication *parent);
    ~KZPController();
    ApplicationState state() { return mState; }
    Q_INVOKABLE  void acceptUserAgreement();
    Q_INVOKABLE  void configured();
    Q_INVOKABLE  void selectSoftwareDriver();
    const QString activeProfile() { return mActiveProfile; }
    const QString applicationVersion() { return APP_VERSION; }
    void setSettingsConfiguration(QString directory, QString profile_name, bool userDirectory);
    Q_INVOKABLE void setPreviewWindow(QObject* window);
    Q_INVOKABLE void loadManifest(QString manifest_file);
    Q_INVOKABLE void loadManifest(QJsonObject manifest_obj);
    static SystemMonitor*  getSystemMonitor(); // Todo: return static global


signals:
    void containerChanged(QQuickItem* container);
    void errorOccurred(QString error_message);


    // Settings UX Signals
    void profileAdded(QString name);
    void profileDataChanged(QString name, QJsonObject data);
    void profileRemoved(QString name);
    void profileChanged(QString profile);
    void stateChanged(KZPController::ApplicationState state);

public slots:
    // Settings Methods
   // void addProfile(QString name);
   // void applyStartupProfile();
   // void selectProfile(QString name);
   // void removeProfile(QString name);
   // void writeSettingsOnExit(QJsonObject current_settings);

    // Handle QApplication logic
    void applicationInitialize();
    void applicationQuiting();
    void detachPreview(bool detach);


protected:
    // Settings Members
    QString                       mSettingsDir;
    QString                       mRootDir;
    QString                       mActiveProfile;
    QJsonObject                   mError;
    QJsonObject                   mProfile;
    AppSettings                   mSettings;
    bool                          mAppliedSettings;
    void         loadProfile();
    void         writeSettingsFile();


    QQmlApplicationEngine*        mUxEngine;
    ApplicationState              mState;
    ApplicationState              mStateBeforeLastError;
    void setUxObjects();

    // Backend Device Interface
    KrakenZInterface*            mController;
    bool createDeviceController();
    bool setSoftwareController();

    // Background Application (Qml on Kraken Device)
    KrakenAppController*         mKrakenAppController;
    bool createKrakenApplicationController();

    // System Tray controller widget
    QIcon                         mApplicationIcon;
    SystemTray*                   mSystemTray;
    void createSystemTray();
    void cleanUp(); // clean it all up

    // Preview Window
    QQuickWindow*                 mPreviewWindow;
    PreviewWindow                 mPreview;

    // Module Manager downloads, and installs modules
    ModuleManager                 mModuleManager;
    void connectModuleManager();

    SystemMonitor*                mHWMonitor;
    void                          initializeHWMonitor();
protected slots:
    // When the UX is changed, this method will control what to do
   // void componentReady();

    void connectToWindow();
    void cleanUpWindow();
    void processBackgroundFrame(QImage frame);
    void moveToBackground();
    void setProfile(QString name);
    bool initializeBackend();
    bool initializeMainWindow();
    void showMainWindow();
    void setMainWindow();
    void receivedInstalledManifests(QVector<QJsonObject>);
    void receivedModuleManifests(QVector<ObjectReply>);
    void releaseMainWindow();

    // module slots

    protected:
    static SystemMonitor*         mSystemMonitor; // TODO: finish connecting in
    QString getSensorPath() {
#ifdef Q_OS_WIN
        return  SharedKeys::LHWMSensors;
#else
        return SharedKeys::LMSensors;
#endif
        return QString{};
    }
};

#endif // KZPCONTROLLER_H
