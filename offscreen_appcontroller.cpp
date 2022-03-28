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

#include "krakenzdriver.h"

static constexpr char RECTQML[] = "import QtQuick 2.15\nRectangle{width:320;height:320;color:\"green\";}";

OffscreenAppController::OffscreenAppController(QObject *controller, QObject *parent)
    : QObject(parent), mController(controller), mContainer(nullptr), mCurrentApp(nullptr), mCurrentComponent(nullptr),
    mOffscreenSurface(nullptr), mGLContext(nullptr), mRenderControl(nullptr), mOffscreenWindow(nullptr), mFBO(nullptr),
    mAppEngine(nullptr), mFrameDelay(76), mFPS(0), mInitialized(false), mActive(true), mAnimationDriver(nullptr),
    mSize(64,64), mDepthSize(32), mStencilSize(8), mAlphaSize(8), mBlueSize(8), mRedSize(8), mGreenSize(8), mPrimaryScreen{nullptr},
    mDelayTimer(new QTimer(parent)), mDPR(1.0)
{
    connect(mDelayTimer, &QTimer::timeout, this, &OffscreenAppController::renderNext);
}

void OffscreenAppController::adjustAnimationDriver()
{
    if(mAnimationDriver) {
        delete mAnimationDriver;
    }
    mAnimationDriver = new AnimationDriver(mFrameDelay);
    mAnimationDriver->install();
    mDelayTimer->setSingleShot(true);
    mDelayTimer->setTimerType(Qt::PreciseTimer);
    mDelayTimer->setInterval(mFrameDelay);
}

void OffscreenAppController::initialize()
{
    if(!mInitialized) {
        mInitialized = true;
        reconfigureSurfaceFormat();
        adjustAnimationDriver();
    }
}


void OffscreenAppController::userComponentReady()
{
    // change to swtich on status
    if(mCurrentComponent->isReady()){
        mCurrentApp = qobject_cast<QQuickItem*>(mCurrentComponent->create());
        //mCurrentApp->setParentItem(mContainer);

        auto errors = mCurrentComponent->errors();
        if(mCurrentComponent->errors().size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
            return;
        }

        if(!mFBO) {
            mFBO = new QOpenGLFramebufferObject(mSize * mDPR, QOpenGLFramebufferObject::CombinedDepthStencil);
            mOffscreenWindow->setRenderTarget(mFBO);
        }
        mCurrentApp->setParentItem(mContainer);
        mCurrentApp->setRotation(-90);
        mCurrentApp->setWidth(mSize.width());
        mCurrentApp->setHeight(mSize.height());

        emit appReady();
        mActive = true;
        renderNext();
    }else {
        qDebug() << "Component failed" << mCurrentComponent->errorString();
        emit qmlFailed(mCurrentComponent->errorString());
        mCurrentComponent->deleteLater();
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

        QQmlComponent baseRect(mAppEngine);
        baseRect.setData(RECTQML, QUrl(":/qml/"));
        auto rectItem = qobject_cast<QQuickItem*>(baseRect.create(mAppEngine->rootContext()));
        rectItem->setParentItem(mContainer);
        mContainer = rectItem;
    }

    mGLContext->makeCurrent(mOffscreenSurface);
    mRenderControl->initialize(mGLContext);
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

void OffscreenAppController::setActive(bool active)
{
    mActive = active;
}

void OffscreenAppController::setAlphaSize(int alpha_size)
{
    if(alpha_size != mAlphaSize) {
        mAlphaSize = alpha_size;
        emit alphaSizeChanged(alpha_size);
        reconfigureSurfaceFormat();
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
    mAnimationDriver->advance();
    if (mActive) {
        QEvent *updateRequest = new QEvent(QEvent::UpdateRequest);
        QCoreApplication::postEvent(this, updateRequest);
    }
}

bool OffscreenAppController::event(QEvent *event)
{
    if(event->type() == QEvent::UpdateRequest)
    {
        mDelayTimer->start();
        event->accept();
        return true;
    }
    return QObject::event(event);
}

void OffscreenAppController::setRedSize(int red_size)
{
    if(red_size != mRedSize) {
        mRedSize = red_size;
        emit redSizeChanged(red_size);
        reconfigureSurfaceFormat();
    }
}

void OffscreenAppController::setGreenSize(int green_size)
{
    if(green_size != mGreenSize) {
        mGreenSize = green_size;
        emit greenSizeChanged(green_size);
        reconfigureSurfaceFormat();
    }
}

void OffscreenAppController::setBlueSize(int blue_size)
{
    if(blue_size != mBlueSize) {
        mBlueSize = blue_size;
        emit blueSizeChanged(blue_size);
        reconfigureSurfaceFormat();
    }
}


void OffscreenAppController::setDepthSize(int depth_size)
{
    if(depth_size != mDepthSize) {
        mDepthSize = depth_size;
        emit depthSizeChanged(depth_size);
        reconfigureSurfaceFormat();
    }
}

void OffscreenAppController::setStencilSize(int stencil_size)
{
    if(stencil_size != mStencilSize) {
        mStencilSize = stencil_size;
        emit stencilSizeChanged(stencil_size);
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
        delete mCurrentApp;
        mCurrentApp = nullptr;
        auto children = mContainer->childItems();
        for( const auto& child: qAsConst(children)){
            delete child;
        }
    }

    mAppEngine->clearComponentCache();
    if(mCurrentComponent){
        delete mCurrentComponent;
    }
    mCurrentComponent = new QQmlComponent(mAppEngine, QUrl(path));
    if(mCurrentComponent->isReady()){
        mCurrentApp = qobject_cast<QQuickItem*>(mCurrentComponent->create());
        auto errors = mCurrentComponent->errors();
        if(mCurrentComponent->errors().size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
            return false;
        }

        if(!mFBO) {
            mFBO = new QOpenGLFramebufferObject(mSize * mDPR, QOpenGLFramebufferObject::CombinedDepthStencil);
            mOffscreenWindow->setRenderTarget(mFBO);
        }

        mCurrentApp->setParentItem(mContainer);
        mCurrentApp->setRotation(-90);
        mCurrentApp->setWidth(mSize.width());
        mCurrentApp->setHeight(mSize.height());

        emit appReady();
        mActive = true;
        loaded = true;
        renderNext();
    }else {
        auto errors = mCurrentComponent->errors();
        if(mCurrentComponent->errors().size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
            qDebug() << "Component failed" << mCurrentComponent->errorString();
            emit qmlFailed(mCurrentComponent->errorString());
            mCurrentComponent->deleteLater();
        }else {
            connect(mCurrentComponent, &QQmlComponent::statusChanged, this, &OffscreenAppController::userComponentReady);
        }
    }
    return loaded;
}


OffscreenAppController::~OffscreenAppController()
{
    mDelayTimer->stop();
    delete mDelayTimer;
    delete mCurrentApp;
    delete mCurrentComponent;
    delete mOffscreenWindow;
    delete mFBO;
    delete mRenderControl;
    delete mGLContext;
    delete mOffscreenSurface;
}

