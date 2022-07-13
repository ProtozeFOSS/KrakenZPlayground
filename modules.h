#ifndef MODULES_H
#define MODULES_H

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QJsonArray>

#include "kzp_coroutines.h"
using namespace KZPCoroutines;

class QQmlApplicationEngine;
class QQuickWindow;

namespace Modules{

namespace ModuleUtility{

    enum ModuleType{
        MODULE = 0,     // qml
        ANIMATION = 1,  // Animation image
        IMAGE = 2,      // static image
        REPO = 3        // modules repo
    };

    struct ModuleManifest{
        QString     name;
        QString     folder;
        QString     url;
        ModuleType  type;
        quint32     version;
        QStringList files;
    };

    struct ImageManifest{
        QString     name;
        QString     entry;
        QString     reference;
    };


    struct Repository{
        QString     name;
        QString     folder;
        QString     url;
        QStringList images;
        QStringList modules;
    };

    enum ActionType{
        DOWNLOAD   = 0,   // Download using NAM
        INSTALL    = 1,   // Install
        FETCH      = 2,   // Fetch manifest/repo (json)
        SEARCH     = 3    // Look at HDD for current installed
    };

    struct Action{
        QString source;
        QString destination;
        QString data;
    };
    typedef QList<Action> ActionList;

    struct InstalledModule{
        ModuleManifest manifest;
        bool           needUpdate = false;
        bool           inUse = false;
    };

    typedef QList<InstalledModule> InstalledList;

    static ModuleManifest createManifestFromJson(QString json_str);
    static ModuleManifest createManifestFromJsObject(QJsonObject obj);

   // static Repository createRepositoryFromJson(QString json_str);
   // static Repository createRepositoryFromJsObject(QJsonObject obj);

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
    Q_INVOKABLE void fetchRepoData(QJsonObject repo);
    Q_INVOKABLE void addRepo(QJsonObject repo);
    Q_INVOKABLE bool removeRepo(QJsonObject repo);
    Q_INVOKABLE void testRepo(QString url);
   // Q_INVOKABLE QJsonArray getRepoImageManifests(QJsonObject repo);

signals:

    //UX signals
    void moduleManagerToggled(bool open);

    void taskStarted(QString file, KZPCoroutines::TaskType type);
    void taskFinished(QString file, KZPCoroutines::TaskType type);
    void installedModulesCheck(QVector<QJsonObject> local_manifests);
    void moduleManifests(const QVector<ObjectReply>& manifests);
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
    void repoImagesReceived(const QVector<ObjectReply>& manifestFiles);
    void repoImageReceived(const QString url, QJsonObject imageManifest);

    // repo module finder
    void repoModuleManifestReceived(QJsonObject module);
    void repoModulesRecieved(QJsonArray modules);

    void repoTest(QJsonObject data);

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

    bool repoExists(const QJsonArray &repos, QString name, QString url);
    bool verifyRepo(QJsonObject &repo, bool &exists);

protected slots:
    void createManager();
    void failedRepoTest();
    void writeRepos(QJsonArray repos);
    void verifyFetchedImageManifests();
    void verifyFetchedImageManifest(const QString url, QJsonObject data);
    void receivedRepoTest();
    void releaseManager();
    void receivedFiles();
    void fetchModulesError(QString error);
    void cleanupLocalModuleCache();
    void cleanupFetchImageFiles();

    //void checkModulesForUpdates();

};

}; // end modules namespace
#endif // MODULES_H
