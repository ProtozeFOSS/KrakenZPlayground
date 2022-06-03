#ifndef MODULES_H
#define MODULES_H

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QJsonArray>
#include <QFuture>

#include "kzp_coroutines.h"
using namespace KZPCoroutines;

class QQmlApplicationEngine;
class QQuickWindow;

namespace Modules{

namespace ModuleUtility{

    enum ModuleType{
        QML = 0,        // qml
        ANIMATION = 1,  // gif
        IMAGE = 2,      // image
        REPOSITORY = 3  // repo
    };

    struct ModuleManifest{
        QString    name;
        QString    folder;
        QString    url;
        ModuleType type;
        quint32    version;
    };

    static ModuleManifest createManifestFromJson(QString json_str);
    static ModuleManifest createManifestFromJsObject(QJsonObject obj);

};

/*******************************************************
 * Modules Manager controls the coroutines
 * used to update and verify the installed Modules
 *
 *
 *
 *
 *
 ******************************************************/

class ModuleManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool moduleManagerOpen READ isManagerOpen WRITE toggleModuleManager NOTIFY moduleManagerToggled)
public:
    explicit ModuleManager(QObject *parent = nullptr);
    ~ModuleManager();
    void checkUpdates();
    void checkModuleUpdates();
    void setRootPath(QString path);
    void fetchModuleManifests(QString task, QVector<QString> manifests);

    Q_INVOKABLE QJsonArray getInstalledModules();
    Q_INVOKABLE QJsonArray getInstalledRepositories();
    Q_INVOKABLE QJsonArray getInstalledImages();
    Q_INVOKABLE void releaseModulesCache();
    Q_INVOKABLE bool isManagerOpen() { return mModuleWindow != nullptr; }
    Q_INVOKABLE bool openExplorerAt(QString path);

    // repository commands
    Q_INVOKABLE QJsonArray getRepoModuleManifests(QJsonObject repo);
   // Q_INVOKABLE QJsonArray getRepoImageManifests(QJsonObject repo);

signals:

    //UX signals
    void moduleManagerToggled(bool open);

    void taskStarted(QString file, KZPCoroutines::TaskType type);
    void taskFinished(QString file, KZPCoroutines::TaskType type);
    void installedModulesCheck(QVector<QJsonObject> local_manifests);
    void moduleManifests(QVector<QJsonObject> manifests);
    void modulesNeedUpdated(QJsonObject modules);


    // from KZP (Local Module Cache)
    void startedCacheLocal();
    void directoryError(QString error_str);
    void moduleFound(QJsonObject module);
    void finishedCachingLocal(QJsonArray data);

    // Image finder
    void lookingForImages();
    void imageFound(QJsonObject image);
    void foundImagesFinished(QJsonArray imagePaths);

    // repo image finder
    void repoImageManifestReceived(QJsonObject manifest);
    void repoImagesReceived(QJsonArray imageManifests);

    // repo module finder
    void repoModuleManifestReceived(QJsonObject module);
    void repoModulesRecieved(QJsonArray modules);

public slots:
     void toggleModuleManager(bool open);

protected:
    QMap<QString, FetchManifestFiles*>     mRequestPool;
    QString                                mRootPath;
    QJsonArray                             mInstalledModules;
    CacheLocalModules*                     mModuleCacher;
    FetchImageFiles*                       mImageFileFetcher;
    FetchRepoManifests*                    mRepoModuleFetcher;
   // CheckExamplesThread*                 mCheckThread;
    bool                                   mManagerOpen;
    QQmlApplicationEngine*                 mModuleEngine;
    QQuickWindow*                          mModuleWindow;

protected slots:
    void createManager();
    void releaseManager();
    void receivedFiles(const QVector<QJsonObject> manifestFiles);
    void fetchModulesError(QString error);
    void cleanupLocalModuleCache();
    void cleanupFetchImageFiles();

    //void checkModulesForUpdates();

};

}; // end modules namespace
#endif // MODULES_H
