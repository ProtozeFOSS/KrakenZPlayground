
#include <QtWidgets/QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "krakenzdriver.h"
#include "krakenimageprovider.h"
#include "offscreen_appcontroller.h"
#include "qusbdevice.h"
#include "systemtray.h"
#include <QStandardPaths>
#include <iostream>
#include "settingsmanager.h"
int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    qmlRegisterUncreatableType<OffscreenAppController>("OffscreenApp", 1, 0, "AppMode", "Cant make this");
    QApplication app(argc, argv);
    app.setApplicationName("Kraken Z Playground");
    KrakenZDriver krakenDevice(&app); // if for some reason you need different PID (driver update?), pass it in here
    QString profile;
    if(argc > 1){
        profile = argv[1];
    }
    auto settingsDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    qDebug() << "Storing settings @" << settingsDir;
    SettingsManager settingsManager(settingsDir, profile, &app);
    OffscreenAppController appController(&krakenDevice, &app);
    appController.setAlphaSize(8);
    appController.setBlueSize(8);
    appController.setDepthSize(32);
    appController.setGreenSize(8);
    appController.setRedSize(8);
    appController.setScreenSize(QSize(320,320));
    appController.setStencilSize(16);
    QObject::connect(&appController, &OffscreenAppController::orientationChanged,
                     &krakenDevice,  &KrakenZDriver::setScreenOrientation);
    SystemTray  systemTray(&app);
    QPixmap iconPixmap(":/images/Droplet.png");
    auto icon = QIcon(iconPixmap);
    systemTray.setIcon(icon);
    auto previewProvider = new KrakenImageProvider(&app);
    previewProvider->setKrakenDevice(&krakenDevice);
    QObject::connect(&appController, &OffscreenAppController::frameReady, previewProvider, &KrakenImageProvider::imageChanged);
    QObject::connect(&systemTray, &SystemTray::profileSelected, &settingsManager, &SettingsManager::selectProfile);
    QQmlApplicationEngine engine;
    engine.addImageProvider("krakenz", previewProvider); // will be owned by the engine
    engine.rootContext()->setContextProperty("AppController", &appController);
    engine.rootContext()->setContextProperty("KrakenZDriver", &krakenDevice);
    engine.rootContext()->setContextProperty("SystemTray", &systemTray);
    engine.rootContext()->setContextProperty("SettingsManager", &settingsManager);
    engine.rootContext()->setContextProperty("KrakenImageProvider", previewProvider);
    engine.rootContext()->setContextProperty("ApplicationPath", app.applicationDirPath());
    systemTray.setEngine(&engine);
    QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    // On created main application Qml
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [&engine, icon,&appController, &systemTray, url](QObject *obj, const QUrl &objUrl) {
        if(objUrl.toString().endsWith(QStringLiteral("clear")))
            return;

        if ((!obj && url == objUrl) ) {
            std::cerr << "Failed to load main.qml\n";
            QCoreApplication::exit(-1);
        }
        else {
            // get QQuickWindow
            bool errored(true);
            auto objects = engine.rootObjects();
            if(objects.size()) {
                auto window{qobject_cast<QQuickWindow*>(objects.at(0))};
                if(window){
                    window->setIcon(icon);
                    auto screen{window->screen()};
                    appController.setPrimaryScreen(screen);
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

    QObject::connect(&app, &QApplication::aboutToQuit, &app, [&settingsManager, &krakenDevice, &appController]{ // on Application close
        QJsonObject profile;
        profile.insert("krakenzdriver", krakenDevice.toJsonProfile());
        profile.insert("appcontroller", appController.toJsonProfile());
        settingsManager.writeSettingsOnExit(profile); // write settings out
    });
    QObject::connect(&settingsManager, &SettingsManager::profileChanged, &app, [&krakenDevice, &appController](int index, QJsonObject data){ // on profile changed
        Q_UNUSED(index)
        krakenDevice.setJsonProfile(data.value("krakenzdriver").toObject());
        appController.setJsonProfile(data.value("appcontroller").toObject());
    });
    QObject::connect(&settingsManager, &SettingsManager::profilesLoaded, &app, [&settingsManager, &systemTray](){ // on profiles loaded
        systemTray.setJsonProfiles(settingsManager.profiles(), settingsManager.currentProfile());
    });
    engine.load(url);

    auto ret_val{app.exec()};
    url = QUrl(QStringLiteral("clear"));
    engine.load(url);
    engine.clearComponentCache();
    engine.collectGarbage();
    return ret_val;
}
