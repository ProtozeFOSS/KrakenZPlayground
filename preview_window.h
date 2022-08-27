#ifndef PREVIEW_WINDOW_H
#define PREVIEW_WINDOW_H

#include <QObject>
#include <QString>
#include <QJsonObject>

class PreviewWindow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal x READ x NOTIFY xChanged MEMBER mX)
    Q_PROPERTY(qreal y READ y NOTIFY yChanged MEMBER mY)
    Q_PROPERTY(qreal height READ height NOTIFY heightChanged MEMBER mHeight)
    Q_PROPERTY(qreal width READ width NOTIFY widthChanged MEMBER mWidth)
    Q_PROPERTY(bool detached READ detached WRITE detachPreview NOTIFY detachChanged MEMBER mDetached)
    Q_PROPERTY(bool movementLocked READ movementIsLocked WRITE lockMovement NOTIFY movementLocked MEMBER mLocked)
    Q_PROPERTY(bool settingsOpen READ settingsOpen WRITE showSettings NOTIFY settingsToggled MEMBER mSettings)
    Q_PROPERTY(QString settingsPath READ settingsPath NOTIFY settingsPathChanged MEMBER mSettingsPath)
    Q_PROPERTY(bool hasSettings READ hasSettings NOTIFY settingsAvailable)

public:

    qreal x() { return mX; }
    qreal y() { return mY; }
    qreal width() { return mWidth; }
    qreal height() { return mHeight; }
    bool  detached() { return mDetached; }
    bool  movementIsLocked() { return mLocked; }
    bool  settingsOpen() { return mSettings; }
    bool  hasSettings() { return (mSettingsPath.size() > 0); }
    QString settingsPath() { return mSettingsPath; }
    void setSettingsPath(QString path);
    void setProfileData(QJsonObject preview);
    QJsonObject profileData();
    PreviewWindow(QObject* parent = nullptr);
    ~PreviewWindow() = default;

    friend class KZPController;

signals:
    void xChanged(qreal x);
    void yChanged(qreal y);
    void widthChanged(qreal width);
    void heightChanged(qreal height);
    void detachChanged(bool detach); // true is detached, false is docked
    void movementLocked(bool locked);
    void settingsToggled(bool open);
    void settingsPathChanged(QString path);
    void settingsAvailable(bool available);

public slots:
    void setPosition(qreal x, qreal y) {
        if(x != mX) {
            mX = x;
            emit xChanged(x);
        }
        if(y != mY) {
            mY = y;
            emit yChanged(y);
        }
    }
    void detachPreview(bool detach)
    {
        if(detach != mDetached) {
            mDetached = detach;
            emit detachChanged(detach);
        }
    }
    void lockMovement(bool lock)
    {
        if(lock != mLocked) {
            mLocked = lock;
            emit movementLocked(lock);
        }
    }
    void showSettings(bool show)
    {
        if(show != mSettings) {
            mSettings = show;
            if(show && !mDetached) {
                detachPreview(true);
            }
            emit settingsToggled(show);
        }
    }

protected:
    qreal                         mX;
    qreal                         mY;
    qreal                         mWidth;
    qreal                         mHeight;
    bool                          mDetached;
    bool                          mLocked;
    bool                          mSettings;
    QString                       mSettingsPath;
};


#endif // PREVIEW_WINDOW_H
