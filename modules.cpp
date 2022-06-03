#include "modules.h"
using namespace Modules;
using namespace Modules::ModuleUtility;

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QDebug>
#include <QStandardPaths>
#include <QQmlContext>
#include <QQuickWindow>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QProcess>

#include "kzp_keys.h"


ModuleManifest createManifestFromJson(QString json)
{
    ModuleManifest ret_val;

    return ret_val;
}



ModuleManager::ModuleManager(QObject *parent)
    : QObject(parent), mModuleCacher{nullptr}, mImageFileFetcher{nullptr},
      mRepoModuleFetcher{nullptr}, mModuleEngine{nullptr}, mModuleWindow{nullptr}
{

}

ModuleManager::~ModuleManager()
{
    if(mModuleCacher) {
        mModuleCacher->terminate();
        delete mModuleCacher;
        mModuleCacher = nullptr;
    }
    mModuleWindow = nullptr;
    delete mModuleEngine;
    mModuleEngine = nullptr;
}

QJsonArray ModuleManager::getInstalledModules()
{
    QJsonArray list;
    if(!mModuleCacher) {
        mModuleCacher = new CacheLocalModules(mRootPath + "/modules", this);
        connect(mModuleCacher, &CacheLocalModules::cacheStarted, this, &ModuleManager::startedCacheLocal);
        connect(mModuleCacher, &CacheLocalModules::directoryError, this, &ModuleManager::directoryError);
        connect(mModuleCacher, &CacheLocalModules::moduleCached, this, &ModuleManager::moduleFound);
        connect(mModuleCacher, &CacheLocalModules::finishedModuleCache, this, &ModuleManager::finishedCachingLocal);
        connect(mModuleCacher, &CacheLocalModules::finished, this, &ModuleManager::cleanupLocalModuleCache);
        mModuleCacher->start();
    } else {
        if(mModuleCacher->isFinished()) {
            list = mModuleCacher->cachedModules();
            emit finishedCachingLocal(list);
        }
    }
    return list;
}


bool ModuleManager::openExplorerAt(QString path)
{
    QDir dir;
    auto filePrefix{QStringLiteral("file:///")};
    if(path.startsWith(filePrefix)) {
        path.remove(filePrefix);
    }
    if(dir.exists(path)) {
        QProcess::startDetached("cmd", {"/c", "start", "File Explorer", path});
        return true;

    }
    return false;
}

bool checkIfAbsolute(QString path)
{
    bool absolute{QDir::isAbsolutePath(path)};
    if(path.indexOf(":") <= 6) {
        absolute = false;
    }
    return absolute;
}

QJsonArray ModuleManager::getInstalledRepositories()
{
    QJsonArray list;
    QString repoFilePath{mRootPath +"/modules/repos.json"};
    QFile f(repoFilePath);
    if(f.open(QFile::ReadOnly)) { // exists
        QJsonParseError e;
        auto doc{QJsonDocument::fromJson(f.readAll(), &e)};
        if(e.error == QJsonParseError::NoError) {
            list = doc.array();
            for( int i{0}; i < list.size(); ++i) {
                auto repoValue{list.at(i)};
                auto repoObject{repoValue.toObject()};
                auto name{repoObject.value(SharedKeys::Name).toString()};
                auto typeInt{repoObject.value(SharedKeys::Type).toInt(-1)};
                auto typeBool{typeInt == 1};
                auto url{repoObject.value(SharedKeys::Url).toString()};
                auto rootUrl{url};
                rootUrl.replace("manifest.json", "");
                auto icon{repoObject.value(SharedKeys::Icon).toString()};
                if(icon.size() && !checkIfAbsolute(icon)) {
                    QString sep{"/"};
                    if(icon.startsWith("/")) {
                        sep.clear();
                    }
                    if(typeBool){
                        icon.prepend(rootUrl);
                    }else {
                        icon.prepend("file:///" + mRootPath + "/modules/" +  name + sep);
                    }
                    repoObject.insert(SharedKeys::Icon, icon);
                }
                list.replace(i, repoObject);
            }
        }
    }else { // create an empty one and return an empty list
        if(f.open(QFile::WriteOnly)) {
            f.write("[]");
        }
    }
    return list;
}

QJsonArray ModuleManager::getRepoModuleManifests(QJsonObject repo)
{
    QJsonArray list;
    auto repoName{repo.value(SharedKeys::Name).toString()};
    if(repoName.size() == 0) {
        return list;
    }
    QString task{QStringLiteral("GetRepo:") + repoName};
    if(!mRequestPool.contains(task)) {
        auto thread = new FetchManifestFiles;
        thread->taskName = task;
//        for(const auto& manifest : qAsConst(manifests)){
//            thread->fileList.append(FileRequest(manifest, QString()));
//        }
        //thread->outputUrl = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +  "/cache/test.zip";
        mRequestPool.insert(task, thread);
        connect(thread, &FetchManifestFiles::errored, this, &ModuleManager::fetchModulesError);
        connect(thread, &FetchManifestFiles::taskStarted, this, &ModuleManager::taskStarted);
        connect(thread, &FetchManifestFiles::taskFinished, this, &ModuleManager::taskFinished);
        connect(thread, &FetchManifestFiles::manifestFilesReceived, this, &ModuleManager::receivedFiles);
        thread->run();
    }
    return list;
}

QJsonArray ModuleManager::getInstalledImages()
{
    QJsonArray list;
    if(!mModuleCacher) {
        mImageFileFetcher = new FetchImageFiles(mRootPath + "/images", this);
        connect(mImageFileFetcher, &FetchImageFiles::lookingForImages, this, &ModuleManager::lookingForImages);
        connect(mImageFileFetcher, &FetchImageFiles::imageFound, this, &ModuleManager::imageFound);
        connect(mImageFileFetcher, &FetchImageFiles::finished, this, &ModuleManager::cleanupFetchImageFiles);
        mImageFileFetcher->start();
    } else {
        if(mImageFileFetcher->isFinished()) {
            list = mImageFileFetcher->getImagePaths();
            emit foundImagesFinished(list);
        }
    }
    return list;
}

void ModuleManager::toggleModuleManager(bool open)
{
    if(mModuleEngine && !open) { // close it
        QTimer::singleShot(1, this, &ModuleManager::releaseManager);
    } else if(!mModuleEngine && open) { // open it
        QTimer::singleShot(1, this, &ModuleManager::createManager);
    }

}

void ModuleManager::createManager()
{
    mModuleEngine = new QQmlApplicationEngine(this);
    if(mModuleEngine) {
        mModuleEngine->rootContext()->setContextProperty("Modules", this);
        connect(mModuleEngine, &QQmlApplicationEngine::objectCreated, this, [this](auto object, auto url){
           mModuleWindow =  qobject_cast<QQuickWindow*>(object);
           if(mModuleWindow) {
               connect(mModuleWindow, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(releaseManager()), Qt::QueuedConnection);
           }
        });
        mModuleEngine->load("file:///" + mRootPath + "/modules/qml/ModuleManager.qml");

    }
}

void ModuleManager::releaseManager()
{
    QTimer::singleShot(10, this, [this](){
        mModuleEngine->disconnect();
        delete mModuleEngine;
        mModuleEngine = nullptr;
        mModuleWindow = nullptr;
        releaseModulesCache();
    });
}

void ModuleManager::cleanupLocalModuleCache()
{
    if(mModuleCacher) {
        mModuleCacher->deleteLater();
        mModuleCacher = nullptr;
    }
}


void ModuleManager::cleanupFetchImageFiles()
{
    if(mImageFileFetcher) {
        mImageFileFetcher->deleteLater();
        mImageFileFetcher = nullptr;
    }
}

void ModuleManager::releaseModulesCache()
{
    if(mModuleCacher) {
        mModuleCacher->disconnect();
        if(!mModuleCacher->isFinished()) {
            cleanupLocalModuleCache();
        } else {
            connect(mModuleCacher, &CacheLocalModules::finished, this, &ModuleManager::cleanupLocalModuleCache);
            mModuleCacher->cancelOperation();
            mModuleCacher = nullptr;
        }
    }
}

void ModuleManager::fetchModuleManifests(QString task, QVector<QString> manifests)
{
    if(!mRequestPool.contains(task)) {
        auto thread = new FetchManifestFiles;
        thread->taskName = task;
        for(const auto& manifest : qAsConst(manifests)){
            thread->fileList.append(FileRequest(manifest, QString()));
        }
        //thread->outputUrl = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +  "/cache/test.zip";
        mRequestPool.insert(task, thread);
        connect(thread, &FetchManifestFiles::errored, this, &ModuleManager::fetchModulesError);
        connect(thread, &FetchManifestFiles::taskStarted, this, &ModuleManager::taskStarted);
        connect(thread, &FetchManifestFiles::taskFinished, this, &ModuleManager::taskFinished);
        connect(thread, &FetchManifestFiles::manifestFilesReceived, this, &ModuleManager::receivedFiles);
        thread->run();
    }
}

void ModuleManager::fetchModulesError(QString error)
{
    qDebug() << "Received File error";
    qDebug() << error;
}

void ModuleManager::receivedFiles(const QVector<QJsonObject> manifestFiles)
{
    auto routine = qobject_cast<FetchManifestFiles*>(sender());
    if(mRequestPool.contains(routine->taskName)) {
        qDebug() << "Received files from " << routine->taskName;
        emit moduleManifests(manifestFiles);
        mRequestPool.take(routine->taskName)->deleteLater();
    }
}


void ModuleManager::setRootPath(QString path)
{
    mRootPath = path;
}


