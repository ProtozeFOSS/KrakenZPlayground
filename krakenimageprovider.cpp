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

void KrakenImageProvider::loadImage(QString file_path)
{
    QImage image;
#ifdef Q_OS_WIN
    file_path = file_path.replace("file:///","");
#else
    filepath = filepath.replace("file://","");
#endif
    if(image.load(file_path)){
        // enforce size
        if(image.width() != 320 || image.height() != 320){
            mImageBuffer = image.scaled(320,320);
        } else {
            mImageBuffer = image;
        }

        QTransform rotation;
        rotation.rotate(270); // rotate to software defined display rotation
        mImageBuffer = mImageBuffer.transformed(rotation);
        if(mDevice) {
            mDevice->setImage(mImageBuffer , mDevice->bucket() ^ 1);
        }
        if(mDisplaying) {
            emit imageReady();
        }
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
