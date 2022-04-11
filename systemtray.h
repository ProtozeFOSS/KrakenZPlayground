#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QObject>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QQuickWindow>
#include <QQmlApplicationEngine>
#include <QJsonArray>

class QMenu;
class QAction;
class SystemTray : public QObject
{
    Q_OBJECT
public:
    explicit SystemTray(QObject *parent = nullptr);
    ~SystemTray();
    void setIcon(QIcon icon);
    void setEngine(QQmlApplicationEngine* engine);
    void setMainWindow(QQuickWindow* window);
    void setJsonProfiles(QJsonArray profiles, QString current);
signals:
    void profileSelected(QString name);
public slots:
    void preventCloseAppWithWindow();
    void setVisible(bool visible = true);
    void activatedSystemTray(QSystemTrayIcon::ActivationReason reason);
protected:
    QQmlApplicationEngine* mMainEngine;
    QQuickWindow*          mMainWindow;
    QSystemTrayIcon*       mTrayIcon;
    QMenu*                 mMenu;
    QAction*               mQuitAction;
    QAction*               mAppBanner;
    QMenu*                 mProfileMenu;
protected slots:
    void handleProfileSelect(bool checked);
    void showMainWindow();

};

#endif // SYSTEMTRAY_H
