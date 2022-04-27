
#include <QtWidgets/QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "krakenzdriver.h"
#include "krakenimageprovider.h"
#include "offscreen_appcontroller.h"
#include "qusbdevice.h"
#include "systemtray.h"
#include <QStandardPaths>
#include <QDir>
#include <iostream>
#include "onscreen_appcontroller.h"
#include "settingsmanager.h"




// HANDLE unix signals (like SIGQUIT for exit on shutdown)
#ifdef Q_OS_LINUX
#include"signal.h"
#include"unistd.h"
static void kill_server(int sig)
{
        QCoreApplication::exit(sig);
}
void handleUnixSignals(const std::vector<int>& quitSignals) {
    // each of these signals calls the handler (quits the QCoreApplication).
    for ( int sig : quitSignals )
        signal(sig, kill_server);
}
#endif

static constexpr char APPLICATION_URI[] = "com.kzp.screens";
static constexpr char OFFAPP[] = "OffscreenApp";
static constexpr char KZPAPP[] = "KZPController";

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

#ifdef Q_OS_LINUX
    handleUnixSignals({ SIGABRT, SIGINT, SIGQUIT, SIGTERM });
#endif

    qmlRegisterUncreatableType<OffscreenAppController>(APPLICATION_URI, 1, 0, OFFAPP, "Cant make this");
    qmlRegisterType<KZPController>(APPLICATION_URI, 1, 0, KZPAPP);
    QApplication app(argc, argv);
    app.setApplicationName("Kraken Z Playground");
    auto driverSelect{new KrakenZDriverSelect{&app}}; // Default is Hardware
    auto krakenDevice { driverSelect->currentDriver()};
    QString profile;
    QString settingsDir;
    if(argc > 1){
        profile = argv[1];
        if(argc > 2) {
            QDir dir;
            if(dir.exists(argv[2])) {
                settingsDir = argv[2];
            }
        }
    }
    if(settingsDir.size() == 0) {
        settingsDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    }
    qDebug() << "Loading settings from" << settingsDir;
    SettingsManager settingsManager(settingsDir, profile, &app);
    auto appController{ new OffscreenAppController(krakenDevice, &app)};
    appController->setAlphaSize(8);
    appController->setBlueSize(8);
    appController->setDepthSize(32);
    appController->setGreenSize(8);
    appController->setRedSize(8);
    appController->setScreenSize(QSize(320,320));
    appController->setStencilSize(16);
    appController->setPrimaryScreen(app.primaryScreen());
    QObject::connect(appController, &OffscreenAppController::orientationChanged,
                     krakenDevice,  &KrakenZInterface::setScreenOrientation);
    SystemTray  systemTray(&app);

    auto previewProvider = new KrakenImageProvider(&app);
    previewProvider->setKrakenDevice(krakenDevice);
    QObject::connect(appController, &OffscreenAppController::frameReady, previewProvider, &KrakenImageProvider::imageChanged);
    QObject::connect(&systemTray, &SystemTray::profileSelected, &settingsManager, &SettingsManager::selectProfile);
    auto engine{new QQmlApplicationEngine(&app)};
    QObject::connect(driverSelect, &KrakenZDriverSelect::driverChanged, engine, [&engine](QObject* driver){
        engine->rootContext()->setContextProperty("KrakenZDriver", driver);
    });
    engine->addImageProvider("krakenz", previewProvider); // will be owned by the engine
    engine->rootContext()->setContextProperty("AppController", appController);
    engine->rootContext()->setContextProperty("SystemTray", &systemTray);
    engine->rootContext()->setContextProperty("SettingsManager", &settingsManager);
    engine->rootContext()->setContextProperty("KrakenImageProvider", previewProvider);
    engine->rootContext()->setContextProperty("ApplicationData", settingsDir);
    engine->rootContext()->setContextProperty("KrakenZDriver", krakenDevice);
    systemTray.setEngine(engine);
    QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    // On created main application Qml
    QObject::connect(engine, &QQmlApplicationEngine::objectCreated,
                     &app, [&engine, &systemTray, &url](QObject *obj, const QUrl &objUrl) {
        if ((!obj && url == objUrl) ) {
            std::cerr << "Failed to load main.qml\n";
            QCoreApplication::exit(-1);
        }
        else {
            QPixmap iconPixmap(":/images/Droplet.png");
            auto icon{new QIcon(iconPixmap)};
            systemTray.setIcon(*icon);
            // get QQuickWindow
            bool errored(true);
            auto objects = engine->rootObjects();
            if(objects.size()) {
                auto window{qobject_cast<QQuickWindow*>(objects.at(0))};
                if(window){
                    if(icon){
                        window->setIcon(*icon);
                    }
                    systemTray.setMainWindow(window);
                    errored = false;
                }
            }
            if(errored){
                std::cerr << "Failed to connect to the Window in main.qml";
                QCoreApplication::exit(-1);
            }
        }
    }, Qt::QueuedConnection);

    QObject::connect(&app, &QApplication::aboutToQuit, &app, [&settingsManager, &krakenDevice, &appController, &engine]{ // on Application close
        auto initialized{krakenDevice->initialized()};
        if(initialized) {
            krakenDevice->setNZXTMonitor();
        }
        appController->closeQmlApplications();
        if(!settingsManager.errored()) {
            QJsonObject profile;
            profile.insert("krakenzdriver", krakenDevice->toJsonProfile());
            profile.insert("appcontroller", appController->toJsonProfile());
            settingsManager.writeSettingsOnExit(profile); // write settings out
        }
        delete appController;
        appController = nullptr;
        delete engine;
        engine = nullptr;
        delete krakenDevice;
        krakenDevice = nullptr;
    });
    QObject::connect(&settingsManager, &SettingsManager::profileChanged, &app, [krakenDevice, appController](int index, QJsonObject data){ // on profile changed
        Q_UNUSED(index)
        krakenDevice->setJsonProfile(data.value("krakenzdriver").toObject());
        appController->setJsonProfile(data.value("appcontroller").toObject());
    });
    QObject::connect(&settingsManager, &SettingsManager::profilesLoaded, &app, [&settingsManager, &systemTray](){ // on profiles loaded
        systemTray.setJsonProfiles(settingsManager.profiles(), settingsManager.currentProfile());
    });
    engine->load(url); // Start main.qml
    auto ret_val{app.exec()};
    delete appController;
    delete engine;
    delete krakenDevice;
    return ret_val;
}
