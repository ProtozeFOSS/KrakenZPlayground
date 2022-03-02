#include "krakenappcontroller.h"
#include <QQmlEngine>

KrakenAppController::KrakenAppController(QQuickItem *parent)
    : QQuickItem(parent), mContainer(nullptr), mCurrentApp(nullptr)
{

}

void KrakenAppController::userComponentReady()
{
    auto component = qobject_cast<QQmlComponent*>(sender());
    // change to swtich on status
    if(component->isReady()){
        mCurrentApp = qobject_cast<QQuickItem*>(component->create());
        mCurrentApp->setParentItem(mContainer ? mContainer:this);
        auto errors = component->errors();
        if(component->errors().size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
        }
        emit appReady();
    }else {
//        auto errors = component->errors();
//        if(component->errors().size()){
//            for(const auto& error: qAsConst(errors)){
//                qDebug() << error;
//            }
//        }
        qDebug() << "Component failed" << component->errorString();
        emit qmlFailed(component->errorString());
        component->deleteLater();
    }
}


bool KrakenAppController::loadQmlFile(QString path)
{
    bool loaded(false);
    auto engine = qmlEngine(this);
    if(mCurrentApp){
        mCurrentApp->setVisible(false);

        delete mCurrentApp;
        mCurrentApp = nullptr;
        auto children = mContainer->childItems();
        for( const auto& child: qAsConst(children)){
            delete child;
        }
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
        emit appReady();
        loaded = true;
    }else {
        auto errors = component.errors();
        if(component.errors().size()){
            for(const auto& error: qAsConst(errors)){
                qDebug() << error;
            }
            qDebug() << "Component failed" << component.errorString();
            emit qmlFailed(component.errorString());
            component.deleteLater();
        }else {
            connect(&component, &QQmlComponent::statusChanged, this, &KrakenAppController::userComponentReady);
        }
    }
    return loaded;
}


