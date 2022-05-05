
#include <QtWidgets/QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "krakenz_driver.h"
#include "preview_provider.h"
#include "kraken_appcontroller.h"
#include "qusbdevice.h"
#include "system_tray.h"
#include <QStandardPaths>
#include <QDir>
#include <iostream>
#include "kzp_controller.h"
#include "settings.h"
#include <QTimer>


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

static constexpr char APPLICATION_URI[] = "com.application.kzp";
static constexpr char OFFAPP[] = "OffscreenApp";
static constexpr char KZPAPP[] = "KZPController";
static constexpr char KZIFACE[] = "KrakenZInterface";
static constexpr char REASON[] = "C++ Instance only";

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

// TODO: Add support for windows OS signals

#ifdef Q_OS_LINUX
    handleUnixSignals({ SIGABRT, SIGINT, SIGQUIT, SIGTERM });
#endif

    qmlRegisterUncreatableType<KrakenAppController>(APPLICATION_URI, 1, 0, OFFAPP, REASON);
    qmlRegisterUncreatableType<KZPController>(APPLICATION_URI, 1, 0, KZPAPP, REASON);
    qmlRegisterUncreatableType<KrakenZInterface>(APPLICATION_URI, 1, 0, KZIFACE, REASON);
    QApplication app(argc, argv);
    app.setApplicationName("Kraken Z Playground");
    KZPController mainController(&app); // Application Business logic

    // Get profile and settings directory from argv
    QString profile;
    QString settingsDir;
    bool userDirectory{false};
    if(argc > 1){
        profile = argv[1];
        if(argc > 2) {
            settingsDir = argv[2];
            userDirectory = true;
        }
    }
    if(settingsDir.size() == 0) {
        userDirectory = false;
        settingsDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);        
    }
    mainController.setSettingsConfiguration(settingsDir, profile, userDirectory);

    // When the Qt Event system starts, initialize the application
    QTimer::singleShot(0, &mainController, &KZPController::applicationInitialize);
    return app.exec();
}
