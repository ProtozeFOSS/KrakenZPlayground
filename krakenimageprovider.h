#ifndef KRAKENIMAGEPROVIDER_H
#define KRAKENIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>

class KrakenZInterface;
class KrakenImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit KrakenImageProvider(QObject* parent = nullptr);
    void setKrakenDevice(KrakenZInterface *device);
    QImage requestImage(const QString &id, QSize *size, const QSize &requested_size) override;

signals:
    void imageReady();

public slots:
    void setDisplayVisible(bool visible);
    void imageChanged(QImage frame);
protected:
    QImage             mImageBuffer;
    KrakenZInterface*     mDevice;
    bool               mDisplaying;
};

#endif // KRAKENIMAGEPROVIDER_H
