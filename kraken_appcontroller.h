#ifndef KRAKEN_APPCONTROLLER_H
#define KRAKEN_APPCONTROLLER_H

#include <QObject>
#include <QSize>
#include <QString>
#include <QDir>
#include <QQmlComponent>
#include <QImage>
#include <QScreen>
#include <QJsonObject>
#include <QAnimationDriver>
#include <QQuickItem>

class SystemMonitor;
class KrakenZInterface;
class QTimer;
class QOpenGLContext;
class QOpenGLFramebufferObject;
class QOffscreenSurface;
class QQuickRenderControl;
class QQuickWindow;
class QQmlApplicationEngine;
class AnimationDriver : public QAnimationDriver
{
public:
    AnimationDriver(int msPerStep) : mStep(msPerStep), mElapsed(0) {}
    void advance() override {
        mElapsed += mStep;
        advanceAnimation();
    }
    qint64 elapsed() const override { return mElapsed; }

protected:
    int mStep;
    qint64 mElapsed;
};

class KrakenAppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem* container  NOTIFY containerChanged MEMBER mContainer)
    Q_PROPERTY(int frameDelay READ frameDelay WRITE setFrameDelay NOTIFY frameDelayChanged MEMBER mFrameDelay)
    // Screen Size
    Q_PROPERTY(QSize screenSize READ screenSize WRITE setScreenSize NOTIFY screenSizeChanged MEMBER mSize)
    Q_PROPERTY(Qt::ScreenOrientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged MEMBER mOrientation);

    // Application and preview control
    Q_PROPERTY(AppMode mode READ mode NOTIFY modeChanged MEMBER mMode)
    Q_PROPERTY(bool drawFPS READ drawFPS WRITE setDrawFPS NOTIFY drawFPSChanged MEMBER mDrawFPS)
    Q_PROPERTY(bool showFPS READ showFPS WRITE setShowFPS NOTIFY showFPSChanged MEMBER mShowFPS)
    Q_PROPERTY(QString loadedPath READ loadedPath NOTIFY loadedPathChanged MEMBER mLoadedPath)
    Q_PROPERTY(bool animationPlaying READ animationPlaying WRITE setAnimationPlaying NOTIFY animationPlayingChanged MEMBER mPlaying)

public:
    KrakenAppController(QObject* preview, KrakenZInterface* controller, SystemMonitor* monitor, QObject* parent = nullptr);
    ~KrakenAppController();
    enum AppMode{ BUILT_IN = -1, STATIC_IMAGE = 0, GIF_MODE = 1, QML_APP = 2};
    Q_ENUM(AppMode)
    AppMode mode() {return mMode; }

    // Public API
    Q_INVOKABLE void loadImage(QString file_path);
    Q_INVOKABLE bool loadQmlFile(QString path);
    Q_INVOKABLE QJsonObject toJsonProfile();
    Q_INVOKABLE void setOrientationFromAngle(int angle);
    Q_INVOKABLE QString getLocalFolderPath(QString path);

    bool  event(QEvent *event) override;
    bool  animationPlaying() { return mPlaying; }
    Q_INVOKABLE bool timerDrawn() { return mActive; }
    QSize screenSize() { return mSize; }
    void  closeQmlApplications();
    int   depthSize() { return mDepthSize; }
    int   stencilSize() { return mStencilSize; }
    int   alphaSize()  { return mAlphaSize; }
    int   blueSize()  { return mBlueSize; }
    int   redSize()  { return mRedSize; }
    int   greenSize() { return mGreenSize; }
    int   frameDelay() { return mFrameDelay; }
    void  setPrimaryScreen(QScreen* screen);
    Qt::ScreenOrientation orientation() { return mOrientation; }
    bool drawFPS() { return mDrawFPS; }
    bool showFPS() { return mShowFPS; }
    void setDepthSize(int depth_size);
    void setStencilSize(int stencil_size);
    void setAlphaSize(int alpha_size);
    void setRedSize(int red_size);
    void setBlueSize(int blue_size);
    void setGreenSize(int green_size);
    void setController(KrakenZInterface* controller);
    QString loadedPath() { return mLoadedPath; }
    void setSystemMonitor(SystemMonitor* monitor);


    // Application Settings API
    Q_INVOKABLE QJsonObject loadAppSettings();
    Q_INVOKABLE void        saveAppSettings(QJsonObject);

signals:
    void containerChanged(QQuickItem* container);
    void animationPlayingChanged(bool animation);
    void appReady();
    void qmlFailed(QString error);
    void frameDelayChanged(int frame_delay);
    void screenSizeChanged(QSize size);
    void fpsChanged(int fps);
    void initialized();
    void draw();
    void frameReady(QImage frame);
    void orientationChanged(Qt::ScreenOrientation orientation);
    void modeChanged(KrakenAppController::AppMode mode);
    void loadGIFPrompt(QString file_path);
    void drawFPSChanged(bool draw_fps);
    void showFPSChanged(bool show_fps);
    void loadedPathChanged(QString path);

public slots:
    void initialize();
    void initializeOffScreenWindow();
    void setBuiltIn(bool loadingGif);
    void setFrameDelay(int frame_delay);
    void setScreenSize(QSize screen_size);
    void setOrientation(Qt::ScreenOrientation orientation, bool updateController = true);
    void setDrawFPS(bool draw_fps);
    void setShowFPS(bool show_fps);
    void setJsonProfile(QJsonObject profile);
    void setAnimationPlaying(bool playing = true);

protected slots:
    void userComponentReady();
    void containerComponentReady();
    void renderNext();

protected:
    QObject*          mPreview;
    KrakenZInterface* mController;
    SystemMonitor*    mMonitor;
    QQuickItem*       mContainer;
    QQuickItem*       mCurrentApp;
    QQmlComponent*    mCurrentComponent;
    QQmlComponent*    mContainerComponent;

    // Offscreen rendering
    QOffscreenSurface*         mOffscreenSurface;
    QOpenGLContext*            mGLContext;
    QQuickRenderControl*       mRenderControl;
    QQuickWindow*              mOffscreenWindow;
    QOpenGLFramebufferObject*  mFBO;
    QQmlApplicationEngine*     mAppEngine;
    Qt::ScreenOrientation      mOrientation;

    // controlling the state
    int mFrameDelay;
    uint8_t mDrawCount;
    bool mInitialized;
    bool mActive;
    AnimationDriver*           mAnimationDriver;

    // Surface format (set to match outgoing display)
    QSize    mSize;
    int      mDepthSize;
    int      mStencilSize;
    int      mAlphaSize;
    int      mBlueSize;
    int      mRedSize;
    int      mGreenSize;
    QObject* mPrimaryScreen;
    QTimer*  mDelayTimer;
    qreal    mDPR;
    AppMode  mMode;
    bool     mDrawFPS;
    bool     mShowFPS;
    bool     mPlaying;
    QString  mLoadedPath;

    void adjustAnimationDriver();
    void createApplication();
    void createContainer();
    void handleComponentErrors();
    void releaseApplication(bool deleteComponent = true);
    void releaseAppEngine();
    void resetAppEngine();
    void setMode(AppMode mode);
    void reconfigureSurfaceFormat();
};

#endif // KRAKEN_APPCONTROLLER_H
