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
#include <QUrl>

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
    if(path.startsWith("http")){
        return true;
    }
    bool absolute{QDir::isAbsolutePath(path)};
    if(path.indexOf(":") <= 6) {
        absolute = false;
    }
    return absolute;
}

QJsonArray ModuleManager::getInstalledRepositories()
{
    QJsonArray list;
    QString repoFilePath{mRootPath +"/modules/installed.json"};
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
                rootUrl = rootUrl.left(rootUrl.lastIndexOf("/") + 1);
                auto icon{repoObject.value(SharedKeys::Icon).toString()};
                if(icon.size() && !checkIfAbsolute(icon)) {
                    QString sep{"/"};
                    if(icon.startsWith("/")) {
                        sep.clear();
                    }
                    if(typeBool && !icon.startsWith(rootUrl)){
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

void ModuleManager::writeRepos(QJsonArray repos)
{
    QString repoFilePath{mRootPath +"/modules/installed.json"};
    QFile f(repoFilePath);
    if(f.open(QFile::WriteOnly)) { // exists
        QJsonDocument doc;
        doc.setArray(repos);
        f.write(doc.toJson());
    }
}


void ModuleManager::fetchRepoData(QJsonObject repo)
{
    auto repoName{repo.value(SharedKeys::Name).toString()};
    auto rurl{repo.value(SharedKeys::Url).toString()};
    if(repoName.size() == 0 || rurl.size() == 0) {
        return;
    }
    QString task{rurl + ":images"};
    if(!mRequestPool.contains(task)) {
        auto imageTask = new FetchManifestFiles;
        imageTask->taskName = task;
        auto images{repo.value(SharedKeys::Images).toArray()};
        for(const auto& jsonImageRef :  qAsConst(images) ){
            auto imagePath{jsonImageRef.toString()};
            if(imagePath.size()) {
                imageTask->fileList.append(FileRequest(imagePath, QString()));
            }
        }
//        for(const auto& manifest : qAsConst(manifests)){
//            thread->fileList.append(FileRequest(manifest, QString()));
//        }
        //thread->outputUrl = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +  "/cache/test.zip";
        mRequestPool.insert(task, imageTask);
        connect(imageTask, &FetchManifestFiles::errored, this, &ModuleManager::fetchModulesError);
        connect(imageTask, &FetchManifestFiles::taskStarted, this, &ModuleManager::taskStarted);
        connect(imageTask, &FetchManifestFiles::objectReceived, this, &ModuleManager::verifyFetchedImageManifest);
        connect(imageTask, &FetchManifestFiles::manifestFilesReceived, this, &ModuleManager::verifyFetchedImageManifests);
        connect(imageTask, &FetchManifestFiles::done, this, [this, imageTask, task](){
            mRequestPool.remove(task);
            imageTask->deleteLater();
        });
        imageTask->run();
    }
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

void ModuleManager::testRepo(QString url)
{
    if(url.size() <=  6) { // not likelt a valid path
        QJsonObject data; // add error
        data.insert(SharedKeys::Url, url);
        data.insert(SharedKeys::ErrorString, url.prepend("Bad path: "));
        emit repoTest(data);
        return; // bad url
    }
    if(url.startsWith(QStringLiteral("http"))) {
        auto thread = new FetchManifestFiles;
        thread->taskName = url;
        thread->fileList.append(FileRequest(url, QString()));
        connect(thread, &FetchManifestFiles::errored, this, &ModuleManager::failedRepoTest);
        connect(thread, &FetchManifestFiles::taskFinished, this, &ModuleManager::taskFinished);
        connect(thread, &FetchManifestFiles::done, thread, [thread](){
            thread->deleteLater();
        });
        connect(thread, &FetchManifestFiles::manifestFilesReceived, this, &ModuleManager::receivedRepoTest);
        thread->run();
    } else { // check if it is a local file
        QDir dir;
        if(dir.exists(url)){
            QFile f(url);
            if(f.open(QFile::ReadOnly)) {
                auto doc = QJsonDocument::fromJson(f.readAll());
                auto data = doc.object();
                if(!data.isEmpty()) {
                    QJsonObject response;
                    response.insert(SharedKeys::Url, url);
                    bool exists{false};
                    if(verifyRepo(data,exists)){
                        response.insert("data", data);
                        if(exists){
                            response.insert("exists", true);
                        }else {
                            response.insert("valid", true);
                        }
                    }
                    emit repoTest(response);
                }
            }
        }
    }
}

void ModuleManager::failedRepoTest()
{
    auto thread = qobject_cast<FetchManifestFiles*>(sender());
    if(thread) {
        QJsonObject data;
        auto ret_val = thread->fileList.at(0);
        data.insert("error", ret_val.first.prepend("Failed to get: "));
        emit repoTest(data);
        delete thread;
        thread = nullptr;
    }
}

void ModuleManager::receivedRepoTest()
{
    auto thread = qobject_cast<FetchManifestFiles*>(sender());
    if(thread) {
        auto repoManifest{thread->getFiles()};
        QJsonObject data;
        if(repoManifest.size() > 0){
            auto reply{repoManifest.at(0)};
            auto repo = reply.second;
            bool exists{false};
            if(verifyRepo(repo, exists)) {
                auto ret_val = repoManifest.at(0);
                data.insert("url", reply.first);
                if(exists) {
                    data.insert("exists", true);
                }else {
                    data.insert("valid", true);
                }
                data.insert("data", repo);
            }else{
                data.insert(SharedKeys::ErrorString, "Failed to get a repository file");
            }
        }else{
            data.insert(SharedKeys::ErrorString, "Failed to get a repository file");
        }
        emit repoTest(data);
    }
}

bool ModuleManager::repoExists(const QJsonArray& repos, QString name, QString url)
{
    bool retval{false};
    for(const auto& jsonValue : repos) {
        auto repo{jsonValue.toObject()};
        auto rname{repo.value(SharedKeys::Name).toString()};
        auto rurl{repo.value(SharedKeys::Url).toString()};
        if(rname.compare(name) == 0 && rurl.compare(url) == 0) {
            retval =  true;
            break;
        }
    }
    return retval;
}

void ModuleManager::verifyFetchedImageManifest(const QString url, QJsonObject data)
{
    auto iname{data.value(SharedKeys::Name).toString()};
    auto baseUrl{url.left(url.lastIndexOf("/"))};
    if(iname.size() > 0 && baseUrl.size() > 0) {
        auto path{data.value(SharedKeys::Path).toString()};
        if(!checkIfAbsolute(path)){
            QString sep("/");
            if(path.startsWith(QStringLiteral("/"))){
                sep.clear();
            }
            if(baseUrl.startsWith(QStringLiteral("http"))){
               path.prepend(baseUrl + sep);
            }else{
               path.prepend("file:///" + baseUrl + sep + iname);
            }
            data.insert(SharedKeys::Icon, path);
        }
    }
    emit repoImageReceived(url, data);
}

void ModuleManager::verifyFetchedImageManifests()
{

    auto thread = qobject_cast<FetchManifestFiles*>(sender());
    if(thread) {
        auto repoManifest{thread->getFiles()};
        for(auto& pair : repoManifest) {
            auto obj{pair.second};
            auto url{pair.first};
            auto iname{obj.value(SharedKeys::Name).toString()};
            auto baseUrl{url.left(url.lastIndexOf("/"))};
            if(iname.size() > 0 && baseUrl.size() > 0) {
                auto path{obj.value(SharedKeys::Path).toString()};
                if(!checkIfAbsolute(path)){
                    QString sep("/");
                    if(path.startsWith(QStringLiteral("/"))){
                        sep.clear();
                    }
                    if(baseUrl.startsWith(QStringLiteral("http"))){
                       path.prepend(baseUrl + sep);
                    }else{
                       path.prepend("file:///" + baseUrl + sep + iname);
                    }
                    obj.insert(SharedKeys::Icon, path);
                }
                pair.second = obj;
            }
        }
        emit repoImagesReceived(repoManifest);
    }
}

bool ModuleManager::verifyRepo(QJsonObject& repo, bool& exists)
{
    qDebug() << repo;
    auto name{repo.value(SharedKeys::Name).toString()};
    auto typeInt{repo.value(SharedKeys::Type).toInt(-1)};
    if(typeInt != ModuleUtility::ModuleType::REPO) {
        return false;
    }
    auto url{repo.value(SharedKeys::Url).toString()};
    auto rootUrl{url};
    rootUrl = rootUrl.left(rootUrl.lastIndexOf("/")+1);
    if(url.size() < 6) {
        return false;
    }
    auto icon{repo.value(SharedKeys::Icon).toString()};
    if(icon.size() && !checkIfAbsolute(icon)) {
        QString sep{"/"};
        if(icon.startsWith("/")) {
            sep.clear();
        }
        if(rootUrl.startsWith("http")){
            icon.prepend(rootUrl);
        }else {
            icon.prepend("file:///" + rootUrl + sep + icon);
        }
        repo.insert(SharedKeys::Icon, icon);
    }
    auto images{repo.value(SharedKeys::Images).toArray()};
    if(images.size() > 0) {
        QJsonArray imagesOut;
        for(int i{0}; i < images.size(); ++i ){
            auto imagePath = images.at(i).toString();

            QString sep{"/"};
            if(imagePath.startsWith("/")) {
                sep.clear();
            }
            if(!checkIfAbsolute(imagePath)){
                if(rootUrl.startsWith("http")){
                    imagePath.prepend(rootUrl);
                } else {
                    imagePath.prepend("file:///" + rootUrl + sep + imagePath);
                }
            }
            imagesOut.append(imagePath);
        }
        repo.remove(SharedKeys::Images);
        repo.insert(SharedKeys::Images, imagesOut);
    }
    auto installed(getInstalledRepositories());
    exists = repoExists(installed, name, url);

    return true;
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

void ModuleManager::addRepo(QJsonObject repo)
{
    auto rname{repo.value(SharedKeys::Name).toString()};
    auto rurl{repo.value(SharedKeys::Url).toString()};
    auto installed{getInstalledRepositories()};
    if(!repoExists(installed , rname, rurl)){
        installed.append(repo);
        writeRepos(installed);
    }
}

bool ModuleManager::removeRepo(QJsonObject repo)
{
    auto installedRepos(getInstalledRepositories());
    auto rname{repo.value(SharedKeys::Name).toString()};
    auto rurl{repo.value(SharedKeys::Url).toString()};
    bool retVal{false};
    for(int i{0}; i < installedRepos.size(); ++i ){
        auto irepo{installedRepos[i].toObject()};
        auto iname{irepo.value(SharedKeys::Name).toString()};
        auto iurl{irepo.value(SharedKeys::Url).toString()};
        if(rname.compare(iname) == 0 && rurl.compare(iurl) == 0){
            installedRepos.removeAt(i);
            writeRepos(installedRepos);
            retVal = true;
            break;
        }
    }
    return retVal;
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
        connect(thread, &FetchManifestFiles::taskFinished, thread, [this, thread, task](){
            mRequestPool.remove(task);
            thread->deleteLater();
        });
        connect(thread, &FetchManifestFiles::manifestFilesReceived, this, &ModuleManager::receivedFiles);
        thread->run();
    }
}

void ModuleManager::fetchModulesError(QString error)
{
    qDebug() << "Received File error";
    qDebug() << error;
}

void ModuleManager::receivedFiles()
{
    auto routine = qobject_cast<FetchManifestFiles*>(sender());
    if(routine) {
        auto manifestFiles{routine->getFiles()};
        if(mRequestPool.contains(routine->taskName)) {
            qDebug() << "Received files from " << routine->taskName;
            emit moduleManifests(manifestFiles);
            mRequestPool.take(routine->taskName)->deleteLater();
        }
    }
}


void ModuleManager::setRootPath(QString path)
{
    mRootPath = path;
}


