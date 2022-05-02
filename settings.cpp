#include "settings.h"
#include <QFile>
#include <QJsonValue>
#include <QJsonDocument>
#include <QDir>
#include "kzp_controller.h"
#include "kzp_keys.h"

typedef KZPController::ApplicationState AppState;


constexpr char SETTINGS_FNAME[] = "settings.json";

//void SettingsManager::applyStartupProfile()
//{
//    auto profileName{mProfileName.size() ? mProfileName:mSettingsObject.value("startProfile").toString()};
//    if(profileName.size()) {
//        auto profiles = mSettingsObject.value("profiles").toArray();
//        auto profileCount = profiles.size();
//        bool notFound(true);
//        for(int index{0}; index < profileCount; ++index){
//            auto profile = profiles.at(index).toObject();
//            auto name = profile.value("name").toString();
//            if(name.compare(profileName) == 0) { // found startup profile
//                mProfileName = name;
//                auto data = profile.value("data").toObject();
//                emit profilesLoaded();
//                emit profileChanged(index, data);
//                notFound = false;
//            }
//        }
//        if(notFound) {
//            mSettingsErrored = true;
//            emit settingsErrored(true);
//        }
//    }
//}



//void SettingsManager::createDefaultSettings()
//{
//    if(mSettingsErrored){
//        return;
//    }
//    QFile settingsFile(mFilePath);
//    if(settingsFile.open(QFile::WriteOnly)) {
//        QJsonDocument doc;
//        doc.setObject(defaultSettingsObject());
//        settingsFile.write(doc.toJson());
//    }
//}

static inline unsigned int countLineNumbers(const QByteArray& data, int& offset)
{
    unsigned int count{0};
    if(offset > data.size()) {
        return 0;
    }
    int lastLineIndex{0};
    for(auto index{0}; index < offset; ++index ){
        if(data.at(index) == '\n'){
            ++count;
            lastLineIndex = index + 1;
        }
    }
    offset -= lastLineIndex;
    return count + 1;
}

QJsonObject Settings::loadSettings(QString directory, QString& profileName, int &state, bool userDirectory)
{
    qDebug() << "Loading settings from" << directory;
    QDir settingsDir;
    QString filePath;
    QJsonObject settings;
    bool directoryExists{false};
    if(!settingsDir.exists(directory)) {
        if(userDirectory) {
            state = AppState::ERROR_SETTINGS_NF;
            settings.insert(TYPE_KEY, state);
            settings.insert(MSG_KEY, "No settings found @: " + directory + QDir::separator() + SETTINGS_FNAME);
            return settings;
        } else {
            if(!settingsDir.mkpath(directory)) {
                qDebug() << "Was not able to create path " << directory << " - settings will not be stored";
                return settings;
            }else {
                directoryExists = true;
            }
        }
    }else {
        directoryExists = true;
    }
    filePath = directory +  QDir::separator() + SETTINGS_FNAME;
    // attempt open and load
    QFile settingsFile(filePath);
    QJsonParseError parseError;
    QByteArray jsonData;
    if(settingsFile.open(QFile::ReadOnly)) {
        jsonData = settingsFile.readAll();
        auto settingsDoc = QJsonDocument::fromJson(jsonData, &parseError);
        if(settingsDoc.isObject() && !settingsDoc.isNull() && parseError.error == QJsonParseError::NoError) {
            settings = settingsDoc.object();
            if(settings.size() != 0) {
                QJsonObject profile;
                profileName = profileName.size() ? profileName:settings.value("startProfile").toString();                
                auto profiles = settings.value("profiles").toArray();
                auto profileCount = profiles.size();
                bool notFound(true);
                if(profileCount && profileName.size()) {
                    for(int index{0}; index < profileCount; ++index){
                        auto profile = profiles.at(index).toObject();
                        auto name = profile.value("name").toString();
                        if(name.compare(profileName) == 0) { // found startup profile
                            profileName = name;
                            settings.insert("activeProfile", profile.value("data").toObject());
                            state = AppState::BACKGROUND;
                            notFound = false; // on success, the settings object is returned
                            break;
                        }
                    }

                }
                if(notFound ) {
                    if(profileCount > 0) {
                        state = AppState::ERROR_PROFILE_NF;
                    } else { // profiles are borked
                        state = AppState::ERROR_PROFILES;                        
                        settings.insert(TYPE_KEY, state);
                        settings.insert(MSG_KEY, "Settings file\ndoes not contain a profiles array");
                        settings.insert(PATH_KEY,"File: " + filePath + "\n");
                        QString errorString("Missing or corrupt 'profiles':[{profile}] member\n");
                        errorString.append("Fix or delete the corrupt file");
                        settings.insert(ERROR_KEY, errorString);
                    }
                }
            }else {
                state = AppState::ERROR_PARSE_SETTINGS;
            }
        } else {
            state = AppState::ERROR_PARSE_SETTINGS;
        }
    }else{
        if(userDirectory) {
            state = AppState::ERROR_SETTINGS_NF;
        } else {
            // create default
            if(directoryExists) {
                state = AppState::FIRST_TIME;
            }
        }
    }

    if(state == AppState::ERROR_PARSE_SETTINGS) { // error occurred, return an error message instead
        settings.insert(TYPE_KEY, state);
        settings.insert(MSG_KEY, "Failed to parse settings.json");
        settings.insert(PATH_KEY,"File: " + filePath + "\n");
        QString errorString("Parse Error\n");
        auto errorCharacter{jsonData.at(parseError.offset-1)};
        auto lineNumber = countLineNumbers(jsonData, parseError.offset);
        errorString.append("Line: ");
        errorString.append(QString::number(lineNumber));
        errorString.append("  character offset: ");
        errorString.append(QString::number(parseError.offset));
        errorString.append(" " + parseError.errorString() + "  '" + errorCharacter + "' ");
        settings.insert(ERROR_KEY, errorString);
    }

    return settings;
}

QJsonObject Settings::defaultProfileObject()
{
    QJsonObject profile;
    profile.insert("name","Default");
    profile.insert("data",QJsonObject());
    return profile;
}

QJsonObject Settings::defaultSettingsObject()
{
    QJsonObject settings;
    settings.insert("profiles",QJsonArray());
    settings.insert("appSettings",QJsonObject());
    return settings;
}



//void SettingsManager::writeSettingsOnExit(QJsonObject current_settings)
//{
//    auto profiles = mSettingsObject.value("profiles").toArray();
//    if(profiles.size() == 0) {
//        // create default profile
//        auto default_profile = defaultProfileObject();
//        default_profile.remove("data");
//        default_profile.insert("data", current_settings);
//        default_profile.insert("name", "Default");
//        profiles.append(default_profile);
//        mSettingsObject.insert("startProfile","Default");
//        mSettingsObject.insert("profiles",profiles);
//        writeCurrentSettings();
//    }else {
//        auto profileCount = profiles.size();
//        for(int index{0}; index < profileCount; ++index){
//            auto profile = profiles.at(index).toObject();
//            auto name = profile.value("name").toString();
//            if(name.compare(mProfileName) == 0) { // found startup profile
//                auto default_profile = defaultProfileObject();
//                default_profile.insert("data", current_settings);
//                default_profile.insert("name", mProfileName);
//                profiles.replace(index,default_profile);
//                mSettingsObject.insert("profiles", profiles);
//                writeCurrentSettings();
//            }
//        }
//    }
//    qDebug() << "Updated settings file @" << mFilePath;
//}
