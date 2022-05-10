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
    static QJsonObject defaultProfileObject();
    static QJsonObject defaultSettingsObject();
    static void writeSettingsFile(QString directory, QJsonObject settings);

    // on updates/changes save profile.
protected:
    Settings(){};

};

#endif // SETTINGS_H
