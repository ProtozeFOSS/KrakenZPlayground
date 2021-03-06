#include "preview_provider.h"
ProxyImageProvider::ProxyImageProvider(QObject *parent) : QObject(parent), QQuickImageProvider(QQuickImageProvider::Image),
    mImageBuffer(320,320,QImage::Format_RGBA8888)
{

}


void ProxyImageProvider::imageChanged(QImage frame)
{
    mImageBuffer = frame;
    emit imageReady();
}



QImage ProxyImageProvider::requestImage(const QString &id, QSize *size, const QSize &requested_size)
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
