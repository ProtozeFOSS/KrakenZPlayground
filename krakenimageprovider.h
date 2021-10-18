#ifndef KRAKENIMAGEPROVIDER_H
#define KRAKENIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>

class KrakenZDriver;
class KrakenImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit KrakenImageProvider(QObject* parent = nullptr);
    void setKrakenDevice(KrakenZDriver* device);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

signals:
    void imageReady();

public slots:
    void imageChanged(QImage frame);

protected:
    QImage             mImageBuffer;
    KrakenZDriver*     mDevice;
};

#endif // KRAKENIMAGEPROVIDER_H
