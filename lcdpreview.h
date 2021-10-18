#ifndef LCDPREVIEW_H
#define LCDPREVIEW_H

#include <QQuickItem>
#include <QImage>
class LCDPreview : public QQuickItem
{
    Q_OBJECT
    // background
    Q_PROPERTY(QQuickItem* background READ background NOTIFY backgroundChanged MEMBER mBackground)
    // image (raw data source)
    Q_PROPERTY(QImage* image READ imageData NOTIFY imageDataChanged MEMBER mImage)
    // circle rectangle
    Q_PROPERTY(QQuickItem* preview READ preview NOTIFY previewChanged MEMBER mPreviewItem)


public:
    LCDPreview(QQuickItem* parent = nullptr);
    ~LCDPreview();

    // property getters
    QQuickItem* background() { return mBackground; };
    QImage*     imageData() { return mImage; };
    QQuickItem* preview() { return mPreviewItem; };

    void classBegin() override;
    void componentComplete() override;

signals:
    void backgroundChanged(QQuickItem* background);
    void imageDataChanged(QImage* image);
    void previewChanged(QQuickItem* preview);

protected:
    QQuickItem* mBackground;
    QImage*     mImage;
    QQuickItem* mPreviewItem;

};

#endif // LCDPREVIEW_H
