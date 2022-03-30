#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QObject>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QQuickWindow>
#include <QQmlApplicationEngine>
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
signals:

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
protected slots:
    void showMainWindow();

};

#endif // SYSTEMTRAY_H
