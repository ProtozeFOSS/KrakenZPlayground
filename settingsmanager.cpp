#include "settingsmanager.h"
#include <QFile>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDir>
#include "krakenzdriver.h"

SettingsManager::SettingsManager(QString directory, QString profile, QObject *parent)
    : QObject{parent}, mProfileName{profile}, mSettingsErrored{false}
{
    QDir settingsDir;
    if(!settingsDir.exists(directory)) {
        if(!settingsDir.mkpath(directory)) {
            qDebug() << "Was not able to create path " << directory << " - settings will not be stored";
        }
    }
    mFilePath = directory;
    mFilePath.append(QDir::separator());
    mFilePath.append("settings.json");
}


void SettingsManager::addProfile(QString name)
{

}

void SettingsManager::applyStartupProfile()
{
    auto profileName{mProfileName.size() ? mProfileName:mSettingsObject.value("startProfile").toString()};
    if(profileName.size()) {
        // seach through the profiles and fine definition for profileName
        qDebug() << "Setting startup profile:" << profileName;
        auto profiles = mSettingsObject.value("profiles").toArray();
        auto profileCount = profiles.size();
        bool notFound(true);
        for(int index{0}; index < profileCount; ++index){
            auto profile = profiles.at(index).toObject();
            auto name = profile.value("name").toString();
            if(name.compare(profileName) == 0) { // found startup profile
                mProfileName = name;
                auto data = profile.value("data").toObject();
                emit profilesLoaded();
                emit profileChanged(index, data);
                notFound = false;
            }
        }
        if(notFound) {
            mSettingsErrored = true;
            emit settingsErrored(true);
        }
    }
}

void SettingsManager::removeProfile(QString name)
{

}

void SettingsManager::createDefaultSettings()
{
    if(mSettingsErrored){
        return;
    }
    QFile settingsFile(mFilePath);
    if(settingsFile.open(QFile::WriteOnly)) {
        QJsonDocument doc;
        doc.setObject(defaultSettingsObject());
        settingsFile.write(doc.toJson());
    }
}

QJsonObject SettingsManager::defaultProfileObject()
{
    QJsonObject profile;
    profile.insert("name","Default");
    profile.insert("data",QJsonObject());
    return profile;
}

QJsonObject SettingsManager::defaultSettingsObject()
{
    QJsonObject settings;
    settings.insert("profiles",QJsonArray());
    settings.insert("appSettings",QJsonObject());
    return settings;
}

bool SettingsManager::loadSettings()
{
    bool loaded{false};
    QFile settingsFile(mFilePath);
    if(settingsFile.open(QFile::ReadOnly)) {
        auto settingsDoc = QJsonDocument::fromJson(settingsFile.readAll());
        if(settingsDoc.isObject() && !settingsDoc.isNull()) {
            mSettingsObject = settingsDoc.object();
            if(mSettingsObject.size() != 0) {
                loaded = true;
            }else {
                mSettingsErrored = true;
                emit settingsErrored(mSettingsErrored);
            }
        } else {
            mSettingsErrored = true;
            emit settingsErrored(mSettingsErrored);
        }
    }
    return loaded;
}


QJsonArray SettingsManager::profiles()
{
    return mSettingsObject.value("profiles").toArray();
}

void SettingsManager::selectProfile(QString name)
{
    // find the profile, then set it as current
    if(name.size()) {
        qDebug() << "Changing profile to " << name;
        auto profiles = mSettingsObject.value("profiles").toArray();
        auto profileCount = profiles.size();
        for(int index{0}; index < profileCount; ++index){
            auto profile = profiles.at(index).toObject();
            auto profileName = profile.value("name").toString();
            if(profileName.compare(name) == 0) { // found startup profile
                mProfileName = name;
                auto data = profile.value("data").toObject();
                emit profileChanged(index, data);
            }
        }
    }
}

void SettingsManager::writeCurrentSettings()
{
    if(mSettingsErrored){
        return;
    }
    QFile settingsFile(mFilePath);
    if(settingsFile.open(QFile::WriteOnly)) {
        QJsonDocument doc;
        doc.setObject(mSettingsObject);
        settingsFile.write(doc.toJson());
    }
}

void SettingsManager::writeSettingsOnExit(QJsonObject current_settings)
{
    auto profiles = mSettingsObject.value("profiles").toArray();
    if(profiles.size() == 0) {
        // create default profile
        auto default_profile = defaultProfileObject();
        default_profile.remove("data");
        default_profile.insert("data", current_settings);
        default_profile.insert("name", "Default");
        profiles.append(default_profile);
        mSettingsObject.insert("startProfile","Default");
        mSettingsObject.insert("profiles",profiles);
        writeCurrentSettings();
    }else {
        auto profileCount = profiles.size();
        for(int index{0}; index < profileCount; ++index){
            auto profile = profiles.at(index).toObject();
            auto name = profile.value("name").toString();
            if(name.compare(mProfileName) == 0) { // found startup profile
                auto default_profile = defaultProfileObject();
                default_profile.insert("data", current_settings);
                default_profile.insert("name", mProfileName);
                profiles.replace(index,default_profile);
                mSettingsObject.insert("profiles", profiles);
                writeCurrentSettings();
            }
        }
    }
    qDebug() << "Updated settings file @" << mFilePath;
}
