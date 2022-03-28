#include "systemtray.h"
#include <QIcon>
#include <QPixmap>
#include <QMenu>
#include <QAction>
#include <QApplication>
SystemTray::SystemTray(QObject *parent)
    : QObject{parent}, mMainEngine{nullptr}, mMainWindow{nullptr}, mTrayIcon{nullptr},
      mMenu{nullptr}, mQuitAction{nullptr}
{
    // create the system tray icon
    mTrayIcon = new QSystemTrayIcon(this);
    mMenu = new QMenu();
    mQuitAction = new QAction("Close", this);
    connect(mQuitAction, &QAction::triggered, this, [](bool checked){
            Q_UNUSED(checked)
            QCoreApplication::exit(0);
    });
    mMenu->addAction(mQuitAction);
    mTrayIcon->setContextMenu(mMenu);
}


void SystemTray::activatedSystemTray(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
        case QSystemTrayIcon::Trigger:
            break;
        case QSystemTrayIcon::DoubleClick: {
            mMainWindow->showNormal();
            break;
        }
        case QSystemTrayIcon::MiddleClick:
            break;
        default:
        ;
    }
}


void SystemTray::preventCloseAppWithWindow()
{
    auto app = qobject_cast<QApplication*>(parent());
    if(app){
        app->setQuitOnLastWindowClosed(false);
    }
}

void SystemTray::setEngine(QQmlApplicationEngine *engine)
{
    if(!mMainEngine){
        mMainEngine = engine;

    }
}

void SystemTray::setMainWindow(QQuickWindow *window)
{
    if(!mMainWindow){
        mMainWindow = window;
        connect(mTrayIcon, &QSystemTrayIcon::activated, this, &SystemTray::activatedSystemTray);
    }
}

void SystemTray::setIcon(QIcon icon)
{

    mTrayIcon->setIcon(icon);
}


void SystemTray::setVisible(bool visible)
{
    mTrayIcon->setVisible(visible);
}


SystemTray::~SystemTray()
{
    delete mMenu;
    delete mQuitAction;
    delete mTrayIcon;
}
