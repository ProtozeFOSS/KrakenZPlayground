#include "krakenappcontroller.h"
#include <QQmlEngine>

KrakenAppController::KrakenAppController(QQuickItem *parent)
    : QQuickItem(parent), mContainer(nullptr), mCurrentApp(nullptr)
{

}

void KrakenAppController::userComponentReady()
{
    auto component = qobject_cast<QQmlComponent*>(sender());
    if(component->isReady()){
        mCurrentApp = qobject_cast<QQuickItem*>(component->create());
        mCurrentApp->setParentItem(mContainer ? mContainer:this);
        auto errors = component->errors();
        if(component->errors().size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
        }
    }
}


bool KrakenAppController::loadQmlFile(QString path)
{
    bool loaded(false);
    auto engine = qmlEngine(this);
    if(mCurrentApp){
        mCurrentApp->setVisible(false);
        mCurrentApp->deleteLater();
    }
    engine->clearComponentCache();
    QQmlComponent component(engine, QUrl(path));
    if(component.isReady()){
//        QVariantMap properties;
//        properties.insert("lcDevice", QVariant::fromValue(deviceObj));
//        QJsonObject deviceInfo;
//        properties.insert("deviceInfo", deviceInfo);
        mCurrentApp = qobject_cast<QQuickItem*>(component.create());
        mCurrentApp->setParentItem(mContainer ? mContainer:this);
        auto errors = component.errors();
        if(component.errors().size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
        }
    }else {
        connect(&component, &QQmlComponent::statusChanged, this, &KrakenAppController::userComponentReady);
    }
    return loaded;
}


