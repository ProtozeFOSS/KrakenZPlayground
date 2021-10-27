#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "krakenzdriver.h"
#include "krakenimageprovider.h"
#include "krakenappcontroller.h"
#include "qusbdevice.h"
int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    qmlRegisterType<KrakenAppController>("com.krakenzplayground.app", 1, 0, "KrakenAppController");
    //qmlRegisterType<QUsbDevice>("com.krakenzplayground.app", 1, 0, "QUsbDevice");
    QGuiApplication app(argc, argv);
    KrakenZDriver krakenDevice(&app); // if for some reason you need different PID (z63?), pass it in here
    auto previewProvider = new KrakenImageProvider(&app);
    previewProvider->setKrakenDevice(&krakenDevice);
    // krakenDevice.setImage(":/images/Droplet.png"); // Cannot use setImage before app.exec
    QQmlApplicationEngine engine;
    engine.addImageProvider("krakenz", previewProvider);
    engine.rootContext()->setContextProperty("KrakenZDriver", &krakenDevice);
    engine.rootContext()->setContextProperty("KrakenPreviewImage", previewProvider);
    engine.rootContext()->setContextProperty("ApplicationPath", app.applicationDirPath());
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
