#include "lcdpreview.h"
#include <QDebug>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQmlEngine>

LCDPreview::LCDPreview(QQuickItem* parent) : QQuickItem(parent),
    mBackground(nullptr), mImage(nullptr), mPreviewItem(nullptr)
{

}


void LCDPreview::classBegin()
{
    auto engine = qmlEngine(this);
    QQmlComponent component(engine, "LCDPreviewDelegate.qml");
    mBackground = qobject_cast<QQuickItem*>(component.create(engine->rootContext()));
}

void LCDPreview::componentComplete()
{
    qDebug() << "Completed LCD Preview component";
}

LCDPreview::~LCDPreview()
{

}
