#include "kraken_appcontroller.h"
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
#include <QQuickWindow>
#include <QQuickRenderControl>
#include <QCoreApplication>
#include <QEvent>
#include <QTimer>
#include <QString>
#include "krakenz_driver.h"
#include "settings.h"
#include "system_monitor.h"
#if QT_VERSION > QT_VERSION_CHECK(6,0,0)
#include <QQuickRenderTarget>
#endif

KrakenAppController::KrakenAppController(QObject* preview, KrakenZInterface *controller, SystemMonitor* monitor,  QObject *parent)
    : QObject{parent}, mPreview{preview}, mController{controller}, mMonitor{monitor}, mContainer{nullptr}, mCurrentApp{nullptr}, mCurrentComponent{nullptr}, mContainerComponent{nullptr},
    mOffscreenSurface(nullptr), mGLContext(nullptr), mRenderControl(nullptr), mOffscreenWindow(nullptr), mFBO(nullptr),
    mAppEngine(nullptr), mOrientation(Qt::LandscapeOrientation), mFrameDelay(160), mDrawCount{0}, mInitialized(false), mActive(false),
    mSize(64,64), mDepthSize(32), mStencilSize(8), mAlphaSize(8), mBlueSize(8), mRedSize(8), mGreenSize(8),
    mPrimaryScreen{nullptr}, mDelayTimer(new QTimer(parent)), mDPR(1.0), mMode(AppMode::STATIC_IMAGE),
    mDrawFPS(false), mShowFPS{false}, mPlaying(false)
{
    connect(mDelayTimer, &QTimer::timeout, this, &KrakenAppController::renderNext);
}

void KrakenAppController::adjustAnimationDriver()
{
    mDelayTimer->setSingleShot(true);
    mDelayTimer->setTimerType(Qt::PreciseTimer);
    mDelayTimer->setInterval(mFrameDelay);
}


void KrakenAppController::closeQmlApplications()
{
    releaseApplication();
    releaseAppEngine();
}

void KrakenAppController::createApplication()
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
    mActive = true;
    setMode(AppMode::QML_APP);
    emit appReady();
    QTimer::singleShot(1, this, &KrakenAppController::renderNext);
}

void KrakenAppController::createContainer()
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
                // handleContainerError();
            }else {
                connect(mContainerComponent, &QQmlComponent::statusChanged, this, &KrakenAppController::containerComponentReady);
            }
        }
    }
}

void KrakenAppController::initialize()
{
    if(!mInitialized) {
        mInitialized = true;
        reconfigureSurfaceFormat();
        adjustAnimationDriver();
        initializeOffScreenWindow();
        createContainer();
    }
}

void KrakenAppController::containerComponentReady()
{
    if(mContainerComponent->isReady()){
        qDebug() << "Container component ready";        
        auto screen = mOffscreenWindow->screen();
        if(screen) {
            mAppEngine->rootContext()->setContextProperty("KrakenScreen", screen);
        }
        if(mPrimaryScreen){
            mAppEngine->rootContext()->setContextProperty("PrimaryScreen", mPrimaryScreen);
        }
        mAppEngine->rootContext()->setContextProperty("PreviewWindow", mPreview);
        mAppEngine->rootContext()->setContextProperty("AppController", this);
        mAppEngine->rootContext()->setContextProperty("DeviceConnection", mController);
        mAppEngine->rootContext()->setContextProperty("SystemMonitor", mMonitor);
        mContainer = qobject_cast<QQuickItem*>(mContainerComponent->create());
        auto errors = mContainerComponent->errors();
        if(errors.size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
            return;
        }
        mContainer->setParentItem(mOffscreenWindow->contentItem());
        emit initialized();
    }else {
        qDebug() << "Component failed" << mContainerComponent->errorString();
        emit qmlFailed(mContainerComponent->errorString());
        mContainerComponent->deleteLater();
    }
}


QString KrakenAppController::getLocalFolderPath(QString path)
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

void KrakenAppController::handleComponentErrors()
{
    auto errors = mCurrentComponent->errors();
    for(const auto& error: qAsConst(errors)){
        qDebug() << error;
    }
    qDebug() << "Component failed" << mCurrentComponent->errorString();
    emit qmlFailed(mCurrentComponent->errorString());
    delete mCurrentComponent;
    mCurrentComponent = nullptr;
    resetAppEngine();
}

void KrakenAppController::initializeOffScreenWindow()
{
    releaseApplication();
    resetAppEngine();
}

void KrakenAppController::loadImage(QString file_path)
{
    mDelayTimer->stop();
    mActive = false;
    mLoadedPath = file_path;
    emit loadedPathChanged(mLoadedPath);
    if(file_path.endsWith(".gif")){
        setMode(AppMode::GIF_MODE);
    } else {
        setMode(AppMode::STATIC_IMAGE);
    }
    renderNext();
}

void KrakenAppController::userComponentReady()
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
        handleComponentErrors();
    }
}

void KrakenAppController::reconfigureSurfaceFormat()
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
        mOffscreenWindow->setFlags(Qt::FramelessWindowHint);
        mOffscreenWindow->setGeometry(0, 0, mSize.width(), mSize.height());
        mContainer = mOffscreenWindow->contentItem();
        mOffscreenWindow->reportContentOrientationChange(Qt::LandscapeOrientation);
    }

    mGLContext->makeCurrent(mOffscreenSurface);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    mRenderControl->initialize(mGLContext);
#else
    mRenderControl->initialize();
#endif

    if(!mFBO) {
        mFBO = new QOpenGLFramebufferObject(mSize * mDPR, QOpenGLFramebufferObject::CombinedDepthStencil);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    mOffscreenWindow->setRenderTarget(mFBO);
#else
    mOffscreenWindow->setRenderTarget(QQuickRenderTarget::fromOpenGLTexture(mFBO->texture(), mFBO->size()));
#endif
    }
}

void KrakenAppController::releaseAppEngine()
{
    if(mAppEngine) {
        delete mAppEngine;
        mAppEngine = nullptr;
    }
}

void KrakenAppController::releaseApplication(bool deleteComponent)
{
    if(mCurrentApp) {
        mDelayTimer->stop();
        mCurrentApp->setVisible(false);
        mCurrentApp->setParentItem(nullptr);
        delete mCurrentApp;
        mCurrentApp =  nullptr;
        if(deleteComponent){
            delete mCurrentComponent;
            mCurrentComponent = nullptr;
        }
        mAppEngine->collectGarbage();
        mAppEngine->clearComponentCache();
    }
}



void KrakenAppController::renderNext()
{
    mRenderControl->polishItems();
    mRenderControl->sync();
    mRenderControl->render();
    mGLContext->functions()->glFlush();
    emit frameReady(mFBO->toImage());
    QEvent *updateRequest = new QEvent(QEvent::UpdateRequest);
    QCoreApplication::postEvent(this, updateRequest);
}

void KrakenAppController::resetAppEngine()
{
    if(mAppEngine) {
        mAppEngine->clearComponentCache();
        mAppEngine->collectGarbage();
        delete mAppEngine;
    }
    mAppEngine = new QQmlApplicationEngine;
    auto screen = mOffscreenWindow->screen();
    if(screen) {
        mAppEngine->rootContext()->setContextProperty("KrakenScreen", screen);
    }
    if(mPrimaryScreen){
        mAppEngine->rootContext()->setContextProperty("PrimaryScreen", mPrimaryScreen);
    }
    mAppEngine->rootContext()->setContextProperty("PreviewWindow", mPreview);
    mAppEngine->rootContext()->setContextProperty("AppController", this);
    mAppEngine->rootContext()->setContextProperty("DeviceConnection", mController);
    mAppEngine->rootContext()->setContextProperty("SystemMonitor", mMonitor);

    if(!mAppEngine->incubationController()) {
        mAppEngine->setIncubationController(mOffscreenWindow->incubationController());
    }
    createContainer();
}

void KrakenAppController::setAnimationPlaying(bool playing)
{
    if(mPlaying != playing) {
        mPlaying = playing;
        emit animationPlayingChanged(mPlaying);
    }
}

void KrakenAppController::setDrawFPS(bool draw_fps)
{
    if(mDrawFPS != draw_fps) {
        mDrawFPS = draw_fps;
        emit drawFPSChanged(draw_fps);
    }
}

void KrakenAppController::setMode(AppMode mode)
{
    mMode = mode;
    if(mMode <= AppMode::STATIC_IMAGE) {
        releaseApplication();
    }
    emit modeChanged(mode);

}
void KrakenAppController::setController(KrakenZInterface *controller)
{
    if(controller != mController) {
        mController = controller;
        if(mAppEngine) {
            mAppEngine->rootContext()->setContextProperty("DeviceConnection", mController);
        }
    }
}

void KrakenAppController::setPrimaryScreen(QScreen *screen)
{
    if(mPrimaryScreen != screen) {
        mPrimaryScreen = screen;
        if(mAppEngine) {
            mAppEngine->rootContext()->setContextProperty("PrimaryScreen", screen);
        }
    }
}

void KrakenAppController::setScreenSize(QSize screen_size)
{
    if(screen_size.width() != mSize.width() || screen_size.height() != mSize.height()) {
        mSize = screen_size;
        emit screenSizeChanged(screen_size);
    }
}


void KrakenAppController::setShowFPS(bool show_fps)
{
    if(mShowFPS != show_fps) {
        mShowFPS = show_fps;
        emit showFPSChanged(show_fps);
    }
}

void KrakenAppController::setBuiltIn(bool loadingGif)
{
    mLoadedPath = loadingGif ?  "1":"2";
    mDelayTimer->stop();
    setMode(AppMode::BUILT_IN);
    mActive = false;
}

void KrakenAppController::setAlphaSize(int alpha_size)
{
    if(alpha_size != mAlphaSize) {
        mAlphaSize = alpha_size;
        reconfigureSurfaceFormat();
    }
}


void KrakenAppController::setOrientation(Qt::ScreenOrientation orientation, bool updateController)
{
    if(mOrientation != orientation) {
        mOrientation = orientation;
        if(updateController) {
            emit orientationChanged(orientation);
        }
    }
}

void KrakenAppController::setOrientationFromAngle(int angle)
{
    mController->setProperty("rotationOffset", angle);
    if(angle >= 90 && angle < 180) {
            setOrientation(Qt::PortraitOrientation, false);
    } else if(angle >= 180 && angle < 270 ) {
            setOrientation(Qt::InvertedLandscapeOrientation, false);
    } else if( angle >= 270) {
            setOrientation(Qt::InvertedPortraitOrientation, false);
    } else {
        setOrientation(Qt::LandscapeOrientation, false);
    }
    if(mMode == STATIC_IMAGE) {
        renderNext();
    }
}

bool KrakenAppController::event(QEvent *event)
{
    if(event->type() == QEvent::UpdateRequest)
    {
        if(mActive) {
            emit draw();
            mDelayTimer->start();
        }
        if(++mDrawCount > 210) {
            mDrawCount = 0;
            mAppEngine->collectGarbage();
        }
        event->accept();
        return true;
    }
    return QObject::event(event);
}

void KrakenAppController::setRedSize(int red_size)
{
    if(red_size != mRedSize) {
        mRedSize = red_size;
        reconfigureSurfaceFormat();
    }
}

void KrakenAppController::setGreenSize(int green_size)
{
    if(green_size != mGreenSize) {
        mGreenSize = green_size;
        reconfigureSurfaceFormat();
    }
}

void KrakenAppController::setBlueSize(int blue_size)
{
    if(blue_size != mBlueSize) {
        mBlueSize = blue_size;
        reconfigureSurfaceFormat();
    }
}


void KrakenAppController::setDepthSize(int depth_size)
{
    if(depth_size != mDepthSize) {
        mDepthSize = depth_size;
        reconfigureSurfaceFormat();
    }
}

void KrakenAppController::setSystemMonitor(SystemMonitor *monitor)
{
    mMonitor = monitor;
    if(mAppEngine) {
        mAppEngine->rootContext()->setContextProperty("SystemMonitor", mMonitor);
    }
}

void KrakenAppController::setStencilSize(int stencil_size)
{
    if(stencil_size != mStencilSize) {
        mStencilSize = stencil_size;
        reconfigureSurfaceFormat();
    }
}

void KrakenAppController::setFrameDelay(int frame_delay)
{
    if(frame_delay != mFrameDelay) {
        mFrameDelay = frame_delay;
        emit frameDelayChanged(frame_delay);
        adjustAnimationDriver();
    }
}

void KrakenAppController::setJsonProfile(QJsonObject profile)
{
    auto loadedPath = profile.value("loadedPath").toString();
    // if loaded path is a manifest file, load manifest
    // else load as mode
    int mode = profile.value("mode").toInt(-2);
    switch(mode){
        case AppMode::BUILT_IN:{
            bool ok{false};
            auto type = loadedPath.toInt(&ok);
            if(ok){
                if(mController) {
                    if(type ==  1) {
                        mController->setBuiltIn(1);
                        setBuiltIn(true);
                    }else {
                        mController->setNZXTMonitor();
                        setBuiltIn(false);
                    }
                }
            }
            break;
        }
        case AppMode::STATIC_IMAGE:
        case AppMode::GIF_MODE: {
            loadImage(loadedPath);
            break;
        }
        case AppMode::QML_APP: {
            loadQmlFile(loadedPath);
            break;
        }

        default:;
    }
    int frameDelay = profile.value("frameDelay").toInt(-2);
    if(frameDelay > 0) {
        setFrameDelay(frameDelay);
    }
}

QJsonObject KrakenAppController::toJsonProfile()
{
    QJsonObject profile;
    profile.insert("mode", mMode);
    profile.insert("loadedPath", mLoadedPath);
    switch(mMode) {
        case QML_APP:{
            profile.insert("frameDelay", mFrameDelay);
            break;
        }
        default:;
    }
    return profile;
}

QJsonObject KrakenAppController::loadAppSettings()
{
    QJsonObject settings;
    if(mLoadedPath.size()) {
        auto dirPath = mLoadedPath;
        if(dirPath.startsWith("file:")) {
            dirPath.remove(0,8);
        }
        auto lastIndex{dirPath.lastIndexOf('/')};
        dirPath = dirPath.left(lastIndex + 1);
        settings = Settings::getRootObject(dirPath);
    }
    return settings;
}

void KrakenAppController::saveAppSettings(QJsonObject settings)
{
    if(mLoadedPath.size()) {
        auto dirPath = mLoadedPath;
        if(dirPath.startsWith("file:")) {
            dirPath.remove(0,8);
        }
        auto lastIndex{dirPath.lastIndexOf('/')};
        dirPath = dirPath.left(lastIndex + 1);
        Settings::writeObject(dirPath, settings);
    }
}

bool KrakenAppController::loadQmlFile(QString path)
{
    bool loaded(false);
    releaseApplication();
    mLoadedPath = path;
    mCurrentComponent = new QQmlComponent(mAppEngine, QUrl(path),QQmlComponent::Asynchronous, this);
    if(mCurrentComponent->isReady()){
        mCurrentApp = qobject_cast<QQuickItem*>(mCurrentComponent->create());
        if(!mCurrentApp || mCurrentComponent->isError()){
            handleComponentErrors();
            return false;
        }
        createApplication();
        loaded = true;
    }else {
        if(mCurrentComponent->isError()){
            handleComponentErrors();
        }else {
            connect(mCurrentComponent, &QQmlComponent::statusChanged, this, &KrakenAppController::userComponentReady);
        }
    }
    return loaded;
}


KrakenAppController::~KrakenAppController()
{
    mDelayTimer->stop();
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
    mMonitor = nullptr;
    mController = nullptr;
    mPreview = nullptr;
}

