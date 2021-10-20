#ifndef KRAKENAPPCONTROLLER_H
#define KRAKENAPPCONTROLLER_H

#include <QQuickItem>

class KrakenAppController : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem* container  NOTIFY containerChanged MEMBER mContainer)
public:
    KrakenAppController(QQuickItem* parent = nullptr);
    Q_INVOKABLE bool loadQmlFile(QString path);
signals:
    void containerChanged(QQuickItem* container);

protected slots:
    void userComponentReady();

protected:
    QQuickItem* mContainer;
    QQuickItem* mCurrentApp;
};

#endif // KRAKENAPPCONTROLLER_H
