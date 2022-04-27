#ifndef KZPCONTROLLER_H
#define KZPCONTROLLER_H

#include <QQuickItem>

class KZPController : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(KZPController)
    Q_PROPERTY(QQuickItem* container READ container WRITE setContainer NOTIFY containerChanged MEMBER mContainer)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorOccurred MEMBER mErrorMessage)
public:
    explicit KZPController(QQuickItem *parent = nullptr);
    QQuickItem* container() { return mContainer; }
    QString errorMessage() { return mErrorMessage; }
signals:
    void containerChanged(QQuickItem* container);
    void errorOccurred(QString error_message);

public slots:
    void setContainer(QQuickItem* container);

protected:
    QQuickItem*    mContainer;
    QString        mErrorMessage;

};
Q_DECLARE_INTERFACE(KZPController, "com.kzp.screens")
#endif // KZPCONTROLLER_H
