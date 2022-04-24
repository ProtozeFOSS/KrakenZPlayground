#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

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
class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool acceptedAgreement READ agreed CONSTANT)
    Q_PROPERTY(bool errored READ errored NOTIFY settingsErrored MEMBER mSettingsErrored)
public:
    explicit SettingsManager(QString directory,QString profile = QStringLiteral("Default"), QObject *parent = nullptr);
    bool agreed(){return loadSettings();}
    bool errored(){ return mSettingsErrored; }
    QJsonArray profiles();
    Q_INVOKABLE QString currentProfile() { return mProfileName;}

public slots:
    void addProfile(QString name);
    void applyStartupProfile();
    void selectProfile(QString name);
    void createDefaultSettings();
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
    QString mFilePath;
    QString mProfileName;
    bool    mSettingsErrored;
    QJsonObject mSettingsObject;

    //void openSettings();
    void writeCurrentSettings();
    QJsonObject defaultProfileObject();
    QJsonObject defaultSettingsObject();
    bool loadSettings();
};

#endif // SETTINGSMANAGER_H
