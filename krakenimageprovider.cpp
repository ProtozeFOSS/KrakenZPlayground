#include "krakenimageprovider.h"
#include "krakenzdriver.h"
#include <QDebug>
KrakenImageProvider::KrakenImageProvider(QObject *parent) : QObject(parent), QQuickImageProvider(QQuickImageProvider::Image),
    mImageBuffer(320,320,QImage::Format_RGBA8888), mDisplaying(true)
{

}

void KrakenImageProvider::setKrakenDevice(KrakenZDriver *device)
{
    mDevice = device;
}

void KrakenImageProvider::imageChanged(QImage frame)
{
    mImageBuffer = frame;
    if(mDevice) {
        mDevice->setImage(mImageBuffer, mDevice->bucket() ^ 1);
    }
    if(mDisplaying) {
        emit imageReady();
    }
}

void KrakenImageProvider::setDisplayVisible(bool visible){
    mDisplaying = visible;
    if(visible) {
        emit imageReady();
    }
}


QImage KrakenImageProvider::requestImage(const QString &id, QSize *size, const QSize &requested_size)
{
    Q_UNUSED(id)
    if(requested_size.height() > 0 && (requested_size.height() != 320 || requested_size.width() != 320)){
        if(size){
            *size = requested_size;
        }
        return mImageBuffer.scaled(requested_size);
    }
    if(size){
        size->setWidth(320);
        size->setHeight(320);
    }
    return mImageBuffer;
}
