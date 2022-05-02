#ifndef KRAKENIMAGEPROVIDER_H
#define KRAKENIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>

class KrakenImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit KrakenImageProvider(QObject* parent = nullptr);
    QImage requestImage(const QString &id, QSize *size, const QSize &requested_size) override;

signals:
    void imageReady();

public slots:
    void imageChanged(QImage frame);
protected:
    QImage             mImageBuffer;
    bool               mDisplaying;
};

#endif // KRAKENIMAGEPROVIDER_H
