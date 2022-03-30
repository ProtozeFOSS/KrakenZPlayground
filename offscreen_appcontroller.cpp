#include "offscreen_appcontroller.h"
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOffscreenSurface>
#include <QQmlEngine>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickRenderControl>
#include <QCoreApplication>
#include <QEvent>
#include <QTimer>
#include <QQuickItem>
#include <QString>

OffscreenAppController::OffscreenAppController(QObject *controller, QObject *parent)
    : QObject(parent), mController(controller), mContainer(nullptr), mCurrentApp(nullptr), mCurrentComponent(nullptr), mContainerComponent{nullptr},
    mOffscreenSurface(nullptr), mGLContext(nullptr), mRenderControl(nullptr), mOffscreenWindow(nullptr), mFBO(nullptr),
    mAppEngine(nullptr), mOrientation(Qt::LandscapeOrientation), mFrameDelay(160), mFPS(0), mInitialized(false), mActive(true),
    mSize(64,64), mDepthSize(32), mStencilSize(8), mAlphaSize(8), mBlueSize(8), mRedSize(8), mGreenSize(8),
    mPrimaryScreen{nullptr}, mDelayTimer(new QTimer(parent)), mDPR(1.0), mMode(AppMode::STATIC_IMAGE), mDrawFPS(false), mPlaying(false)
{
    connect(mDelayTimer, &QTimer::timeout, this, &OffscreenAppController::renderNext);
}

void OffscreenAppController::adjustAnimationDriver()
{
    mDelayTimer->setSingleShot(true);
    mDelayTimer->setTimerType(Qt::PreciseTimer);
    mDelayTimer->setInterval(mFrameDelay);
}

void OffscreenAppController::createApplication()
{
    auto items = mContainer->childItems();
    if(items.size()) {
        auto appContainer = items.at(0);
        mCurrentApp->setParentItem(appContainer);
    } else {
        mCurrentApp->setParentItem(mContainer);
    }
    mCurrentApp->setWidth(mSize.width());
    mCurrentApp->setHeight(mSize.height());
    emit appReady();
    mActive = true;
    setMode(AppMode::QML_APP);
    emit modeChanged(mMode);
    renderNext();
}

void OffscreenAppController::createContainer()
{
    if(!mContainerComponent) {
        mContainerComponent = new QQmlComponent(mAppEngine, QUrl("qrc:/qml/KrakenZContainer.qml"));
        if(mContainerComponent->isReady()) {
            mContainer = qobject_cast<QQuickItem*>(mContainerComponent->create());
            mContainer->setParentItem(mOffscreenWindow->contentItem());
        } else {
            auto errors = mContainerComponent->errors();
            if(mContainerComponent->errors().size()){
                for(const auto& error: qAsConst(errors)){
                    qDebug() << error;
                }
                qDebug() << "Component failed" << mContainerComponent->errorString();
                emit qmlFailed(mContainerComponent->errorString());
                mContainerComponent->deleteLater();
                mContainerComponent = nullptr;
            }else {
                connect(mContainerComponent, &QQmlComponent::statusChanged, this, &OffscreenAppController::containerComponentReady);
            }
        }
    }
}

void OffscreenAppController::initialize()
{
    if(!mInitialized) {
        mInitialized = true;
        reconfigureSurfaceFormat();
        adjustAnimationDriver();
    }
}

void OffscreenAppController::containerComponentReady()
{
    if(mContainerComponent->isReady()){
        qDebug() << "Container component ready";
        mContainer = qobject_cast<QQuickItem*>(mContainerComponent->create());
        auto errors = mContainerComponent->errors();
        if(errors.size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
            return;
        }
        mContainer->setParentItem(mOffscreenWindow->contentItem());
    }else {
        qDebug() << "Component failed" << mContainerComponent->errorString();
        emit qmlFailed(mContainerComponent->errorString());
        mContainerComponent->deleteLater();
    }
}

QString OffscreenAppController::getLocalFolderPath(QString path)
{
    QString out_str;
    if(!path.startsWith(QStringLiteral("file:/"))) {
#ifdef Q_OS_WIN
      out_str = "file:///" +  path;
#else
      out_str = "file://" +  path;
#endif
    }
    else {
        return path;
    }
    return out_str;
}

void OffscreenAppController::loadImage(QString file_path)
{
    mDelayTimer->stop();
    if(file_path.endsWith(".gif")){
        mLoadedPath = file_path;
        emit loadedPathChanged(mLoadedPath);
        mActive = true;
        setMode(AppMode::GIF_MODE);
        emit modeChanged(mMode);
        return;
    }
#ifdef Q_OS_WIN
    file_path = file_path.replace("file:///","");
#else
    file_path = file_path.replace("file://","");
#endif
    QImage image;
    if(image.load(file_path)){
        mLoadedPath = file_path;
        // enforce size
        if(image.width() != 320 || image.height() != 320){
            image = image.scaled(320,320);
        }
        QVariant rot{mController->property("rotationOffset")};
        if(rot.isValid()) {
            QTransform rotation;
            rotation.rotate(-1*rot.toInt()); // rotate to software defined display rotation
            image = image.transformed(rotation);
        }
        setMode(AppMode::STATIC_IMAGE);
        emit frameReady(image);
    }
}

void OffscreenAppController::userComponentReady()
{
    // change to swtich on status
    if(mCurrentComponent->isReady()){
        mCurrentApp = qobject_cast<QQuickItem*>(mCurrentComponent->create());
        auto errors = mCurrentComponent->errors();
        if(mCurrentComponent->errors().size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
            delete mCurrentComponent;
            mCurrentComponent = nullptr;
            return;
        }
        createApplication();
    }else {
        qDebug() << "Component failed" << mCurrentComponent->errorString();
        emit qmlFailed(mCurrentComponent->errorString());

        delete mCurrentComponent;
        mCurrentComponent = nullptr;
    }
}

void OffscreenAppController::reconfigureSurfaceFormat()
{
    // Gaurd statement
    if(!mInitialized)
        return;
    // run the reconfigure surface format routine here.
    QSurfaceFormat format;
    format.setStencilBufferSize(mStencilSize);
    format.setBlueBufferSize(mBlueSize);
    format.setRedBufferSize(mRedSize);
    format.setGreenBufferSize(mGreenSize);
    format.setAlphaBufferSize(mAlphaSize);
    format.setDepthBufferSize(mDepthSize);

    if(!mGLContext) {
        mGLContext = new QOpenGLContext;
    }
    mGLContext->setFormat(format);
    mGLContext->create();

    if(!mOffscreenSurface) {
        mOffscreenSurface = new QOffscreenSurface;
    } else {
        mOffscreenSurface->destroy();
    }
    mOffscreenSurface->setFormat(format);
    if(mOffscreenSurface->format() != format){
        qDebug() << "Failed to set format";
        qDebug() << "Requested format:";
        qDebug() << format;
        qDebug() << mOffscreenSurface->format();
    }
    mOffscreenSurface->create();

    if(!mRenderControl) {
        mRenderControl = new QQuickRenderControl(this);
    }

    if(!mOffscreenWindow) {
        mOffscreenWindow = new QQuickWindow(mRenderControl);
        mOffscreenWindow->setGeometry(0, 0, mSize.width(), mSize.height());
        mContainer = mOffscreenWindow->contentItem();
        mOffscreenWindow->reportContentOrientationChange(Qt::LandscapeOrientation);
    }

    if(!mAppEngine) {
        mAppEngine = new QQmlApplicationEngine;
        auto screen = mOffscreenWindow->screen();
        if(screen) {
            mAppEngine->rootContext()->setContextProperty("KrakenScreen", screen);
        }
        if(mPrimaryScreen){
            mAppEngine->rootContext()->setContextProperty("PrimaryScreen", mPrimaryScreen);
        }
        mAppEngine->rootContext()->setContextProperty("AppController", this);
        mAppEngine->rootContext()->setContextProperty("KrakenZDriver", mController);

        if(!mAppEngine->incubationController()) {
            mAppEngine->setIncubationController(mOffscreenWindow->incubationController());
        }
    }
    mGLContext->makeCurrent(mOffscreenSurface);
    mRenderControl->initialize(mGLContext);

    if(!mFBO) {
        mFBO = new QOpenGLFramebufferObject(mSize * mDPR, QOpenGLFramebufferObject::CombinedDepthStencil);
        mOffscreenWindow->setRenderTarget(mFBO);
    }

    createContainer();
    mActive = true;
}

void OffscreenAppController::setAnimationPlaying(bool playing)
{
    if(mPlaying != playing) {
        mPlaying = playing;
        emit animationPlayingChanged(mPlaying);
    }
}

void OffscreenAppController::setDrawFPS(bool draw_fps)
{
    if(mDrawFPS != draw_fps) {
        mDrawFPS = draw_fps;
        emit drawFPSChanged(draw_fps);
    }
}

void OffscreenAppController::setMode(AppMode mode)
{
    if(mode != mMode) {
        mMode = mode;
        emit modeChanged(mode);
    }
}

void OffscreenAppController::setPrimaryScreen(QScreen *screen)
{
    if(mPrimaryScreen != screen) {
        mPrimaryScreen = screen;
        if(mAppEngine) {
            mAppEngine->rootContext()->setContextProperty("PrimaryScreen", screen);
        }
    }
}

void OffscreenAppController::setScreenSize(QSize screen_size)
{
    if(screen_size.width() != mSize.width() || screen_size.height() != mSize.height()) {
        mSize = screen_size;
        emit screenSizeChanged(screen_size);
    }
}

void OffscreenAppController::setBuiltIn()
{
    mDelayTimer->stop();
    setMode(AppMode::BUILT_IN);
    mActive = false;
}

void OffscreenAppController::setAlphaSize(int alpha_size)
{
    if(alpha_size != mAlphaSize) {
        mAlphaSize = alpha_size;
        reconfigureSurfaceFormat();
    }
}


void OffscreenAppController::setOrientation(Qt::ScreenOrientation orientation)
{
    if(mOrientation != orientation) {
        mOrientation = orientation;
        emit orientationChanged(orientation);
        if(mMode == AppMode::STATIC_IMAGE) {
            QImage image;
            if(image.load(mLoadedPath)){
                // enforce size
                if(image.width() != 320 || image.height() != 320){
                    image = image.scaled(320,320);
                }
                QVariant rot{mController->property("rotationOffset")};
                if(rot.isValid()) {
                    QTransform rotation;
                    rotation.rotate(-1*rot.toInt()); // rotate to software defined display rotation
                    image = image.transformed(rotation);
                }
                setMode(AppMode::STATIC_IMAGE);
                emit frameReady(image);
            }
        }
    }
}

void OffscreenAppController::setOrientationFromAngle(int angle)
{
    switch(angle){
        case 90:
            setOrientation(Qt::PortraitOrientation);
            break;
        case 180:
            setOrientation(Qt::InvertedLandscapeOrientation);
            break;
        case 270:
            setOrientation(Qt::InvertedPortraitOrientation);
            break;
        case 0:
        default: setOrientation(Qt::LandscapeOrientation);
    }
}

void OffscreenAppController::renderNext()
{
    mRenderControl->polishItems();
    mRenderControl->sync();
    mRenderControl->render();
    mGLContext->functions()->glFlush();
    ++mFPS;
    emit frameReady(mFBO->toImage());
    QEvent *updateRequest = new QEvent(QEvent::UpdateRequest);
    QCoreApplication::postEvent(this, updateRequest);
}

bool OffscreenAppController::event(QEvent *event)
{
    if(event->type() == QEvent::UpdateRequest)
    {
        if(mMode == AppMode::QML_APP) {
            mDelayTimer->start();
        }
        event->accept();
        return true;
    }
    return QObject::event(event);
}

void OffscreenAppController::setRedSize(int red_size)
{
    if(red_size != mRedSize) {
        mRedSize = red_size;
        reconfigureSurfaceFormat();
    }
}

void OffscreenAppController::setGreenSize(int green_size)
{
    if(green_size != mGreenSize) {
        mGreenSize = green_size;
        reconfigureSurfaceFormat();
    }
}

void OffscreenAppController::setBlueSize(int blue_size)
{
    if(blue_size != mBlueSize) {
        mBlueSize = blue_size;
        reconfigureSurfaceFormat();
    }
}


void OffscreenAppController::setDepthSize(int depth_size)
{
    if(depth_size != mDepthSize) {
        mDepthSize = depth_size;
        reconfigureSurfaceFormat();
    }
}

void OffscreenAppController::setStencilSize(int stencil_size)
{
    if(stencil_size != mStencilSize) {
        mStencilSize = stencil_size;
        reconfigureSurfaceFormat();
    }
}

void OffscreenAppController::setFrameDelay(int frame_delay)
{
    if(frame_delay != mFrameDelay) {
        mFrameDelay = frame_delay;
        emit frameDelayChanged(frame_delay);
        adjustAnimationDriver();
    }
}

bool OffscreenAppController::loadQmlFile(QString path)
{
    bool loaded(false);
    if(mCurrentApp){
        mCurrentApp->setVisible(false);
        mCurrentApp->setParentItem(nullptr);
        delete mCurrentApp;
        delete mCurrentComponent;
        mCurrentApp =  nullptr;
        mCurrentComponent = nullptr;
        mAppEngine->collectGarbage();
        mAppEngine->trimComponentCache();
    }

    mCurrentComponent = new QQmlComponent(mAppEngine, QUrl(path));
    if(mCurrentComponent->isReady()){
        mCurrentApp = qobject_cast<QQuickItem*>(mCurrentComponent->create());
        auto errors = mCurrentComponent->errors();
        if(mCurrentComponent->errors().size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
            emit qmlFailed(mCurrentComponent->errorString());
            delete mCurrentComponent;
            mCurrentComponent = nullptr;
            return false;
        }
        createApplication();
        loaded = true;
    }else {
        auto errors = mCurrentComponent->errors();
        if(mCurrentComponent->errors().size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
            qDebug() << "Component failed" << mCurrentComponent->errorString();
            emit qmlFailed(mCurrentComponent->errorString());
            delete mCurrentComponent;
            mCurrentComponent = nullptr;
        }else {
            connect(mCurrentComponent, &QQmlComponent::statusChanged, this, &OffscreenAppController::userComponentReady);
        }
    }
    return loaded;
}


OffscreenAppController::~OffscreenAppController()
{
    mDelayTimer->stop();
    mAppEngine->load(":/clear");
    mAppEngine->clearComponentCache();
    mAppEngine->collectGarbage();
    delete mDelayTimer;
    delete mCurrentApp;
    delete mContainer;
    delete mCurrentComponent;
    delete mContainerComponent;
    delete mOffscreenWindow;
    delete mFBO;
    delete mRenderControl;
    delete mGLContext;
    delete mOffscreenSurface;
    delete mAppEngine;
}

