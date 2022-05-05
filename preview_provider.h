#ifndef PREVIEW_PROVIDER_H
#define PREVIEW_PROVIDER_H

#include <QQuickImageProvider>
#include <QImage>

class ProxyImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit ProxyImageProvider(QObject* parent = nullptr);
    QImage requestImage(const QString &id, QSize *size, const QSize &requested_size) override;

signals:
    void imageReady();

public slots:
    void imageChanged(QImage frame);

protected:
    QImage             mImageBuffer;
    bool               mDisplaying;
};

#endif // PREVIEW_PROVIDER_H
