#include "settingsmanager.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDir>
#include "krakenzdriver.h"

SettingsManager::SettingsManager(QString directory, QString profile, QObject *parent)
    : QObject{parent}, mProfileName(profile)
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
                emit profileChanged(index, data);
                notFound = false;
            }
        }
        if(notFound) {
            if(mProfileName.compare("Default") != 0) {
                qDebug() << "Profile" << mProfileName << "was not found, attempting to load Default";
                mProfileName.clear();
                applyStartupProfile();
            } else { // something funky about profiles, dump them
                qDebug() << "Error Applying profile, even the default one";
                createDefaultSettings();
            }
        }
    }
}

void SettingsManager::removeProfile(QString name)
{

}

void SettingsManager::createDefaultSettings()
{
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
        if(settingsDoc.isObject()) {
            mSettingsObject = settingsDoc.object();
            //if(checkValidSettings()) {
            loaded = true;
        }
    }
    return loaded;
}


void SettingsManager::writeCurrentSettings()
{
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
