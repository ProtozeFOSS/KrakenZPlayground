
#include <QtWidgets/QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "krakenzdriver.h"
#include "krakenimageprovider.h"
#include "offscreen_appcontroller.h"
#include "qusbdevice.h"
#include "systemtray.h"
#include <QProcess>
#include <iostream>


int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    // Get Settings location
    // One Version is the application another is the runner

    QApplication app(argc, argv);
    app.setApplicationName("Kraken Z Playground");
    KrakenZDriver krakenDevice(&app); // if for some reason you need different PID (z63?), pass it in here

    OffscreenAppController appController(&krakenDevice, &app);
    appController.setAlphaSize(8);
    appController.setBlueSize(8);
    appController.setDepthSize(32);
    appController.setGreenSize(8);
    appController.setRedSize(8);
    appController.setScreenSize(QSize(320,320));
    appController.setStencilSize(16);
    appController.initialize();
    SystemTray  systemTray(&app);
    QPixmap iconPixmap(":/images/Droplet.png");
    auto icon = QIcon(iconPixmap);
    systemTray.setIcon(icon);
    auto previewProvider = new KrakenImageProvider(&app);
    previewProvider->setKrakenDevice(&krakenDevice);
    QObject::connect(&appController, &OffscreenAppController::frameReady, previewProvider, &KrakenImageProvider::imageChanged);
    QQmlApplicationEngine engine;
    engine.addImageProvider("krakenz", previewProvider);
    engine.rootContext()->setContextProperty("AppController", &appController);
    engine.rootContext()->setContextProperty("KrakenZDriver", &krakenDevice);
    engine.rootContext()->setContextProperty("SystemTray", &systemTray);
    engine.rootContext()->setContextProperty("KrakenImageProvider", previewProvider);
    engine.rootContext()->setContextProperty("ApplicationPath", app.applicationDirPath());
    systemTray.setEngine(&engine);
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [&engine, icon,&appController, &systemTray, url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            std::cerr << "Failed to load main.qml\n";
            QCoreApplication::exit(-1);
        }
        else {
            // get QQuickWindow
            bool errored(true);
            auto objects = engine.rootObjects();
            if(objects.size()) {
                auto window = qobject_cast<QQuickWindow*>(objects.at(0));
                if(window){
                    window->setIcon(icon);
                    auto screen = window->screen();
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
    engine.load(url);

    return app.exec();
}
