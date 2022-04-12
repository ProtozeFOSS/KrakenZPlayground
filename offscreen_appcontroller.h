#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QSize>
#include <QString>
#include <QDir>
#include <QQmlComponent>
#include <QImage>
#include <QScreen>
#include <QJsonObject>
#include <QAnimationDriver>

class KrakenZDriver;
class QQuickItem;
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

class OffscreenAppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem* container  NOTIFY containerChanged MEMBER mContainer)
    // Qml "Animation" frameRate, ideally matches near draw rate
    Q_PROPERTY(int frameDelay READ frameDelay WRITE setFrameDelay NOTIFY frameDelayChanged MEMBER mFrameDelay)
    // Screen Size
    Q_PROPERTY(QSize screenSize READ screenSize WRITE setScreenSize NOTIFY screenSizeChanged MEMBER mSize)
    Q_PROPERTY(Qt::ScreenOrientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged MEMBER mOrientation);
    // Screen Format
    Q_PROPERTY(int currentFPS READ currentFPS NOTIFY fpsChanged MEMBER mFPS)
    Q_PROPERTY(AppMode mode READ mode NOTIFY modeChanged MEMBER mMode)
    Q_PROPERTY(bool drawFPS READ drawFPS WRITE setDrawFPS NOTIFY drawFPSChanged MEMBER mDrawFPS)
    Q_PROPERTY(QString loadedPath READ loadedPath NOTIFY loadedPathChanged MEMBER mLoadedPath)
    Q_PROPERTY(bool animationPlaying READ animationPlaying WRITE setAnimationPlaying NOTIFY animationPlayingChanged MEMBER mPlaying)

public:
    OffscreenAppController(KrakenZDriver* controller, QObject* parent = nullptr);
    ~OffscreenAppController();
    enum AppMode{ BUILT_IN = -1, STATIC_IMAGE = 0, GIF_MODE = 1, QML_APP = 2};
    Q_ENUM(AppMode)
    AppMode mode() {return mMode; }
    Q_INVOKABLE void loadImage(QString file_path);
    Q_INVOKABLE bool loadQmlFile(QString path);
    Q_INVOKABLE QJsonObject toJsonProfile();
    bool  event(QEvent *event) override;
    bool  animationPlaying() { return mPlaying; }
    QSize screenSize() { return mSize; }
    int   currentFPS() { return mFPS; }
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
    void setDepthSize(int depth_size);
    void setStencilSize(int stencil_size);
    void setAlphaSize(int alpha_size);
    void setRedSize(int red_size);
    void setBlueSize(int blue_size);
    void setGreenSize(int green_size);
    Q_INVOKABLE void setOrientationFromAngle(int angle);
    Q_INVOKABLE QString getLocalFolderPath(QString path);
    QString loadedPath() { return mLoadedPath; }

signals:
    void containerChanged(QQuickItem* container);
    void animationPlayingChanged(bool animation);
    void appReady();
    void qmlFailed(QString error);
    void frameDelayChanged(int frame_delay);
    void screenSizeChanged(QSize size);
    void fpsChanged(int fps);
    void draw();
    void frameReady(QImage frame);
    void orientationChanged(Qt::ScreenOrientation orientation);
    void modeChanged(OffscreenAppController::AppMode mode);
    void loadGIFPrompt(QString file_path);
    void drawFPSChanged(bool draw_fps);
    void loadedPathChanged(QString path);

public slots:
    void initialize();
    void initializeOffScreenWindow();
    void setBuiltIn(bool loadingGif);
    void setFrameDelay(int frame_delay);
    void setScreenSize(QSize screen_size);
    void setOrientation(Qt::ScreenOrientation orientation, bool updateController = true);
    void setDrawFPS(bool draw_fps);
    void setJsonProfile(QJsonObject profile);
    void setAnimationPlaying(bool playing = true);

protected slots:
    void userComponentReady();
    void containerComponentReady();
    void renderNext();

protected:
    KrakenZDriver* mController;
    QQuickItem*    mContainer;
    QQuickItem*    mCurrentApp;
    QQmlComponent* mCurrentComponent;
    QQmlComponent* mContainerComponent;

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
    int mFPS;
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
    QTimer*  mStatusTimer;
    qreal    mDPR;
    AppMode  mMode;
    bool     mDrawFPS;
    bool     mPlaying;
    QString  mLoadedPath;

    void adjustAnimationDriver();
    void createApplication();
    void createContainer();
    void handleComponentErrors();
    void releaseApplication(bool deleteComponent = false);
    void resetAppEngine();
    void setMode(AppMode mode);
    void reconfigureSurfaceFormat();
};

#endif // APPCONTROLLER_H
