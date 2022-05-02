#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

/*****************************************************************************
 *  Settings Manager performs CRUD for profiles, and CRUR for the
 *  settings file (replace not delete)
 *
 * If the settings file is missing, it will be created.
 * Base settings file is essentially "empty".
 *
 * If on close there is no profile, one will be created ("Default") and the
 * current settings written.
 *
 * If the settings file becomes corrupted, it just gets replaced
 * with a default.
 *
 * There is no limit on profiles but the application will optimize
 * for handfuls (<10).
 *
 ************************************************************/
class Settings
{
public:

    static QJsonObject loadSettings(QString directory, QString& profileName, int& state, bool userDirectory);
    static QJsonObject defaultProfileObject();
    static QJsonObject defaultSettingsObject();
    static void writeSettingsFile(QString directory, QJsonObject settings);

public slots:
    void addProfile(QString name);
    void applyStartupProfile();
    void selectProfile(QString name);
    void removeProfile(QString name);
    void writeSettingsOnExit(QJsonObject current_settings);

signals:
    void settingsErrored(bool errored);
    void profilesLoaded();
    void profileAdded(QString name);
    void profileChanged(int index, QJsonObject data);
    void profileRemoved(QString name);

    // on updates/changes save profile.
protected:
    Settings(){};

};

#endif // SETTINGS_H
