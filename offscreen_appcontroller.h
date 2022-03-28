#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <QSize>
#include <QString>
#include <QDir>
#include <QFuture>
#include <QQmlComponent>
#include <QImage>
#include <QScreen>
class QQuickItem;
class QTimer;
class QOpenGLContext;
class QOpenGLFramebufferObject;
class QOffscreenSurface;
class QQuickRenderControl;
class QQuickWindow;
class QQmlApplicationEngine;
#include <QAnimationDriver>
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
    //Q_PROPERTY(Qt::ScreenOrientation orientation READ orientation WRITE setOrientation MEMBER mOrientation)
    // Screen Format
    Q_PROPERTY(int depthSize READ depthSize WRITE setDepthSize NOTIFY depthSizeChanged MEMBER mDepthSize)
    Q_PROPERTY(int stencilSize READ stencilSize WRITE setStencilSize NOTIFY stencilSizeChanged MEMBER mStencilSize)
    Q_PROPERTY(int alphaSize READ alphaSize WRITE setAlphaSize NOTIFY alphaSizeChanged MEMBER mAlphaSize)
    Q_PROPERTY(int blueSize READ blueSize WRITE setBlueSize NOTIFY blueSizeChanged MEMBER mBlueSize)
    Q_PROPERTY(int redSize READ redSize WRITE setRedSize NOTIFY redSizeChanged MEMBER mRedSize)
    Q_PROPERTY(int greenSize READ greenSize WRITE setGreenSize NOTIFY greenSizeChanged MEMBER mGreenSize)
    Q_PROPERTY(int currentFPS READ currentFPS NOTIFY fpsChanged MEMBER mFPS)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged MEMBER mActive)


public:
    OffscreenAppController(QObject* controller, QObject* parent = nullptr);
    ~OffscreenAppController();
    Q_INVOKABLE bool loadQmlFile(QString path);
    void  initialize();
    bool  event(QEvent *event) override;
    bool  isActive() { return mActive; }
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

signals:
    void containerChanged(QQuickItem* container);
    void activeChanged(bool active);
    void appReady();
    void qmlFailed(QString error);
    void frameDelayChanged(int frame_delay);
    void screenSizeChanged(QSize size);
    void depthSizeChanged(int depth_size);
    void stencilSizeChanged(int depth_size);
    void alphaSizeChanged(int alpha_size);
    void redSizeChanged(int red_size);
    void greenSizeChanged(int green_size);
    void blueSizeChanged(int blue_size);
    void fpsChanged(int fps);
    void frameReady(QImage frame);

public slots:
    void setActive(bool active);
    void setFrameDelay(int frame_delay);
    void setScreenSize(QSize screen_size);
    void setDepthSize(int depth_size);
    void setStencilSize(int stencil_size);
    void setAlphaSize(int alpha_size);
    void setRedSize(int red_size);
    void setBlueSize(int blue_size);
    void setGreenSize(int green_size);

protected slots:
    void userComponentReady();
    void renderNext();

protected:
    QObject* mController;
    QQuickItem*    mContainer;
    QQuickItem*    mCurrentApp;
    QQmlComponent* mCurrentComponent;

    // Offscreen rendering
    QOffscreenSurface*         mOffscreenSurface;
    QOpenGLContext*            mGLContext;
    QQuickRenderControl*       mRenderControl;
    QQuickWindow*              mOffscreenWindow;
    QOpenGLFramebufferObject*  mFBO;
    QQmlApplicationEngine*     mAppEngine;

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
    qreal    mDPR;

    void adjustAnimationDriver();
    void reconfigureSurfaceFormat();
};

#endif // APPCONTROLLER_H
