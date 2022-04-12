#include "systemtray.h"
#include <QIcon>
#include <QPixmap>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QStyle>
#include <QJsonValue>
#include <QJsonObject>

SystemTray::SystemTray(QObject *parent)
    : QObject{parent}, mMainEngine{nullptr}, mMainWindow{nullptr}, mTrayIcon{nullptr},
      mMenu{nullptr}, mQuitAction{nullptr}, mAppBanner{nullptr}, mProfileMenu{nullptr}
{
    // create the system tray icon
    mTrayIcon = new QSystemTrayIcon(this);
    mMenu = new QMenu();
    mProfileMenu = new QMenu("Profile");
    mQuitAction = new QAction("Close", mMenu);
    connect(mQuitAction, &QAction::triggered, this, [](bool checked){
            Q_UNUSED(checked)
            QCoreApplication::exit(0);
    });
    mAppBanner = new QAction("Kraken Z3",mMenu);
    mAppBanner->setIcon(QIcon(":/images/Peyton.png"));
    mAppBanner->setToolTip("App Information");
    connect(mAppBanner, &QAction::triggered, this, &SystemTray::showMainWindow);
    mQuitAction->setObjectName("closeAction");
    mMenu->addAction(mAppBanner);
    mMenu->addSeparator();
    mProfileMenu->setStyleSheet("QMenu{color: white; background-color: rgb(69,69,69); margin: 2px;}\nQMenu::item:default:selected {background-color:red;}\n QMenu::item {padding: 2px 25px 2px 20px; border: 1px solid transparent;}");
    mMenu->addMenu(mProfileMenu);
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

void SystemTray::handleProfileSelect(bool checked)
{
    auto profile = qobject_cast<QAction*>(sender());
    auto active{mProfileMenu->activeAction()};
    if(profile != active ) {
        emit profileSelected(profile->text());
        mProfileMenu->setActiveAction(profile);
    }
}

void SystemTray::setJsonProfiles(QJsonArray profiles, QString current)
{
    auto profileCount{profiles.size()};
    for( auto index{0}; index < profileCount; ++index) {
        auto jsonValue{profiles.at(index)};
        QJsonObject profileObject;
        if(jsonValue.type() == QJsonValue::Array) {
            auto wrappedArray = jsonValue.toArray();
            if(wrappedArray.size()) {
                profileObject = wrappedArray.at(0).toObject();
            } else {
                return;
            }
        } else {
            profileObject = jsonValue.toObject();
        }
        QString profileName;
        QJsonObject data;
        if(profileObject.isEmpty()) { // bug on linux the object is interpreted as array... grrr
            profileName = jsonValue[0].toString();
            data = jsonValue[1].toObject();
        } else {
            profileName = profileObject.value("name").toString();
            data = profileObject.value("data").toObject();
        }
        auto profileAction{new QAction(profileName,mProfileMenu)};
        profileAction->setText(profileName);
        mProfileMenu->addAction(profileAction);
        if(profileName.compare(current) == 0){
            mProfileMenu->setActiveAction(profileAction);
        }
        connect(profileAction, &QAction::triggered, this, &SystemTray::handleProfileSelect);
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
    delete mProfileMenu;
    delete mMenu;
    mTrayIcon->hide();
    delete mTrayIcon;

}
