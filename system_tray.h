#ifndef SYSTEM_TRAY_H
#define SYSTEM_TRAY_H

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
    void setJsonProfiles(QJsonArray profiles, QString current);
signals:
    void profileSelected(QString name);
    void showMainWindow();

public slots:
    void setVisible(bool visible = true);
    void activatedSystemTray(QSystemTrayIcon::ActivationReason reason);

protected:
    QSystemTrayIcon*       mTrayIcon;
    QMenu*                 mMenu;
    QAction*               mQuitAction;
    QAction*               mAppBanner;
    QMenu*                 mProfileMenu;

protected slots:
    void handleProfileSelect(bool checked);

};

#endif // SYSTEM_TRAY_H
