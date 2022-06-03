#include "preview_window.h"

PreviewWindow::PreviewWindow(QObject* parent) :
    QObject{parent}, mX{0}, mY{0}, mWidth{320}, mHeight{320},
    mDetached{false}, mLocked{false}, mSettings{false}
{}

void PreviewWindow::setProfileData(QJsonObject preview)
{
    auto lValue = preview.value("locked");
    if(!lValue.isNull()) {
        mLocked = lValue.toBool();
    }
    auto dValue = preview.value("detached");
    if(!dValue.isNull()) {
        mDetached = dValue.toBool();
    }
    auto xValue = preview.value("x");
    if(!xValue.isNull()) {
        mX = xValue.toDouble();
    }
    auto yValue = preview.value("y");
    if(!yValue.isNull()) {
        mY = yValue.toDouble();
    }
    auto width = preview.value("width");
    if(!width.isNull()) {
        mWidth = width.toDouble();
    }
    auto height = preview.value("height");
    if(!height.isNull()) {
        mHeight = height.toDouble();
    }
}

QJsonObject PreviewWindow::profileData()
{
    QJsonObject preview;
    preview.insert("x", mX);
    preview.insert("y", mY);
    preview.insert("width", mWidth);
    preview.insert("height", mHeight);
    preview.insert("detached", mDetached);
    preview.insert("locked", mLocked);
    return preview;
}

void PreviewWindow::setSettingsPath(QString path)
{
    if(path.size() > 0 && !path.startsWith("file:///")){
        path.prepend("file:///");
    }
    if(mSettingsPath.compare(path) != 0) {
        mSettingsPath = path;
        emit settingsPathChanged(mSettingsPath);
        emit settingsAvailable((mSettingsPath.size() > 0));
    }
}
