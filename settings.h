#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

/*****************************************************************************
 *  Settings Class is a convenience wrapper for setting related static
 *  functions.
 ****************************************************************************/
class Settings
{
public:

    static QJsonObject loadSettings(QString directory, QString& profileName, int& state, bool userDirectory);
    static QString getSettingsPath(QString directory);
    static QJsonObject getRootObject(QString directory);
    static void        writeObject(QString directory, QJsonObject object);
    // on updates/changes save profile.
protected:
    Settings(){};

};

#endif // SETTINGS_H
