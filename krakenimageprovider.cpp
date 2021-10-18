#include "krakenimageprovider.h"
#include "krakenzdriver.h"
#include <QDebug>
KrakenImageProvider::KrakenImageProvider(QObject *parent) : QObject(parent), QQuickImageProvider(QQuickImageProvider::Image),
    mImageBuffer(320,320,QImage::Format_RGBA8888)
{

}

void KrakenImageProvider::setKrakenDevice(KrakenZDriver *device)
{
    mDevice = device;
    if(mDevice){
        QObject::connect(mDevice, &KrakenZDriver::imageTransfered, this, &KrakenImageProvider::imageChanged);
    }
}

void KrakenImageProvider::imageChanged(QImage frame)
{
    mImageBuffer = frame;
    emit imageReady();
}

QImage KrakenImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    qDebug() << "Image Requested: " << id;

    if(requestedSize.height() > 0 && (requestedSize.height() != 320 || requestedSize.width() != 320)){
        qDebug() << "Requesting scaled";
        if(size){
            *size = requestedSize;
        }
        return mImageBuffer.scaled(requestedSize);
    }
    if(size){
        size->setWidth(320);
        size->setHeight(320);
    }
    return mImageBuffer;
}
