#include "systemtray.h"
#include <QIcon>
#include <QPixmap>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QStyle>
SystemTray::SystemTray(QObject *parent)
    : QObject{parent}, mMainEngine{nullptr}, mMainWindow{nullptr}, mTrayIcon{nullptr},
      mMenu{nullptr}, mQuitAction{nullptr}, mAppBanner{nullptr}
{
    // create the system tray icon
    mTrayIcon = new QSystemTrayIcon(this);
    mMenu = new QMenu();
    mQuitAction = new QAction("Close", this);
    connect(mQuitAction, &QAction::triggered, this, [](bool checked){
            Q_UNUSED(checked)
            QCoreApplication::exit(0);
    });
    mAppBanner = new QAction("Kraken Z3",this);
    mAppBanner->setIcon(QIcon(":/images/Peyton.png"));
    mAppBanner->setToolTip("App Information");
    connect(mAppBanner, &QAction::triggered, this, &SystemTray::showMainWindow);
    mQuitAction->setObjectName("closeAction");
    mMenu->addAction(mAppBanner);
    mMenu->addSeparator();
    mMenu->addAction(mQuitAction);
    mMenu->setDefaultAction(mQuitAction);
    mMenu->setStyleSheet("QMenu{color: white; background-color: rgb(69,69,69); margin: 2px;}\nQMenu::item:default:selected {background-color:red;}\n QMenu::item {padding: 2px 25px 2px 20px; border: 1px solid transparent;}");
    mMenu->setBaseSize(100, 200);
    mTrayIcon->setContextMenu(mMenu);
}


void SystemTray::activatedSystemTray(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
        case QSystemTrayIcon::Trigger:
            showMainWindow();
            break;
        case QSystemTrayIcon::DoubleClick: {
            showMainWindow();
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


void SystemTray::showMainWindow()
{
    mMainWindow->showNormal();
    mMainWindow->raise();  // for MacOS
    mMainWindow->setVisible(true);
#ifdef Q_OS_WIN
    mMainWindow->setWindowState(Qt::WindowState::WindowActive);
#endif
}

SystemTray::~SystemTray()
{
    delete mAppBanner;
    delete mQuitAction;
    delete mMenu;
    mTrayIcon->hide();
    delete mTrayIcon;

}
