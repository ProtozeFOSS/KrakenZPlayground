#ifndef KZP_COROUTINES_H
#define KZP_COROUTINES_H

#include <QObject>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include "kzp_keys.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDataStream>
#include <QMetaObject>
#include <QDirIterator>
#include <QImageReader>
#include <QQmlProperty>


/************************* "Coroutines"  *************************************
* This section describes a set of object that encapsulate asynchronous events
* in a non Qt environment these would need to be threaded for efficiency
* and to prevent block the GUI. QNetworkAccessManager was designed
* to be used on the main thread in using non-blocking signals/slots.
* The actual coroutines use the Qt Concurent api.
****************************************************************************/
namespace KZPCoroutines{

/*********************** Convenience and helpers **************************************/
constexpr char USER_AGENT[]{"Kraken Z Playground - (ProtozeFoss)"};

typedef enum {
    TERMINAL = 404,
    RECOVERABLE = 400
} ErrorAction;

typedef enum {

}ErrorType;

typedef enum {
    FETCH_MANIFEST = 800,
    FETCH_MODULE = 900,
    INSTALL_MODULE = 1000,
    CHECK_MODULE = 2000,
}TaskType;

static QJsonObject generateErrorObject(QNetworkReply::NetworkError error)
{
    QJsonObject obj;
    //TODO: Fill out based on error reproted
    return obj;
}

typedef QPair<QString, QString> FileRequest;

class FileFetcher : public QObject
{
    Q_OBJECT
public:
    QString filePath;
    FileFetcher(QObject* parent = nullptr, QByteArray userAgent = USER_AGENT) : QObject(parent), mProcessed{0}, mReplyCount{0}, mNam{this}, mUserAgent{userAgent} {}

    void fetchFiles(QVector<FileRequest>& requests)
    {
        mRequests = &requests;
        queueRequests();
    }
    void cancel()
    {

    }

signals:
    void fetchFileError(QJsonObject error);
    void fetchingFile(QString file);
    void fileProgress(quint8 percent);
    void fileReceived(QString path, QByteArray data);
    void finishedRequests();

protected slots:
    void receivedData()
    {
        qDebug() << "Received data from file request";
    }

    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
    {
        if(reply) {
            qDebug() << "SSL Error on File Fetcher";
            reply->ignoreSslErrors(errors);
        }
    }
    void requestErrored(QNetworkReply::NetworkError error)
    {
        emit fetchFileError(generateErrorObject(error));
        qDebug() << "Error on File Fetcher " << error;
    }
    void replyFinished()
    {
        auto reply{qobject_cast<QNetworkReply*>(sender())};
        if(reply->error() == QNetworkReply::NoError) {
            auto path = reply->request().url().path();
            ++mProcessed;
            emit fileReceived(path,reply->readAll());
        }
        reply->deleteLater();
        --mReplyCount;
        if(mProcessed == mRequests->size()) {
            // Finished;
            emit finishedRequests();
        }else {
            queueRequests();
        }
    }

protected:
    void createNetworkRequest(const FileRequest& request)
    {
        bool writeFile{request.second.size() > 0};
        QNetworkRequest network_request{request.first};
        network_request.setRawHeader("User-Agent", mUserAgent);
        network_request.setMaximumRedirectsAllowed(5);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        network_request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#else

#endif
        connect(&mNam, &QNetworkAccessManager::sslErrors, this, &FileFetcher::sslErrors);
        emit fetchingFile(request.first);
        ++mReplyCount;
        auto reply = mNam.get(network_request);
        if(reply->error() != QNetworkReply::NoError) {
            qDebug() << "Errored" << reply->errorString();

        }
        connect(reply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError error){
            auto reply = qobject_cast<QNetworkReply*>(sender());
            qDebug() << "File Fetcher Errored: " << reply->errorString();

        });
        if(writeFile) {
            reply->setProperty(SharedKeys::FilePath.toStdString().c_str(), request.second);
            connect(reply, &QNetworkReply::readyRead, this, &FileFetcher::receivedData);
        }else {
            connect(reply, &QNetworkReply::finished, this, &FileFetcher::replyFinished, Qt::QueuedConnection);
        }

    }
    void queueRequests()
    {
        auto remaining{mRequests->size() - mProcessed};
        auto replySlots = qMin(6 - mReplyCount, remaining);
        for(int index{0}; index < replySlots; ++index)
        {
            // create a newtwork request for the file
            auto request = mRequests->at(mProcessed + index);
            createNetworkRequest(request);
        }

    }
    QString                 mWritePath;
    QVector<FileRequest>*   mRequests;
    int                     mProcessed;
    int                     mReplyCount;
    QNetworkAccessManager   mNam;
    QByteArray              mUserAgent;
    QDataStream             mFileStream;
};



static bool checkValidManifestFile(const QString& dirPath, QJsonObject& data)
{
    auto success{true};
    auto type{data.value(SharedKeys::Type).toBool()};
    auto name{data.value(SharedKeys::Name).toString()};
    if((success = name.size() == 0)) {
        return success;
    }
    auto url{data.value(SharedKeys::Url).toString()};
    auto rootUrl{url};
    auto rootPath{"file:///" + dirPath};
    if(rootUrl.size()) {
        rootUrl.replace("manifest.json", "");
        data.insert(SharedKeys::Path,rootUrl);
    }else {
        data.insert(SharedKeys::Path,rootPath);
    }
    auto entry{data.value(SharedKeys::Entry).toString()};
    if((success = entry.size() != 0)) {
        if(!QDir::isAbsolutePath(entry)){
            if (type){
                entry.prepend(rootUrl);
            } else {
                entry.prepend(rootPath);
            }
            data.remove(SharedKeys::Entry);
            data.insert(SharedKeys::Entry, entry);
        }
    }else {
        return success;
    }
    auto icon{data.value(SharedKeys::Icon).toString()};
    if((success = icon.size() != 0)) {
        if(!QDir::isAbsolutePath(icon)){
            if(type){
                icon.prepend(rootUrl);
            }   else {
                icon.prepend(rootPath);
            }
            data.remove(SharedKeys::Icon);
            data.insert(SharedKeys::Icon, icon);
        }
    }else {
        return success;
    }
    auto files{data.value(SharedKeys::Files).toArray()};
    QJsonArray newFiles;
    bool useNewFiles{false};
    for(const auto & file :  files) {
        auto filePath = file.toString();
        if((success = filePath.size() != 0)) {
            if(!QDir::isAbsolutePath(filePath)){
                if (type) {
                    filePath.prepend(rootUrl);
                } else {
                    filePath.prepend(rootPath);
                }
                newFiles.append(filePath);
                useNewFiles = true;
            }
        }
    }
    if(useNewFiles) {
        data.remove(SharedKeys::Files);
        data.insert(SharedKeys::Files, newFiles);
    }
    return success;
}

// Starts at the top level Modules directory and recursively looks for valid manifests
class CacheLocalModules : public QThread
{
    Q_OBJECT

signals:
    void cacheStarted(); // notify to clear and show signal
    void moduleCached(QJsonObject module);
    void finishedModuleCache(QJsonArray modules);
    void invalidModule(QJsonObject obj, QString name, QString path);
    void directoryError(QString error);

public:
    CacheLocalModules(QString module_directory, QObject* parent = nullptr) : QThread{parent}, mDirectory{module_directory} {}
    void run() override
    {
        QTimer::singleShot(0, this, &CacheLocalModules::startCaching);
        exec(); // Built in event loop now?
        QMetaObject::invokeMethod(this, "finishedModuleCache", Qt::QueuedConnection, Q_ARG(QJsonArray, mData));
    }
    QString directory(){ return mDirectory; }
    QJsonArray cachedModules() { return mData; }

public slots:
    void cancelOperation()
    {
        qDebug() << "Requesting cancel Cache Local Modules";
        exit();
    }

protected slots:
    void startCaching()
    {
        QMetaObject::invokeMethod(this, &CacheLocalModules::cacheStarted, Qt::QueuedConnection);
        QDir dir(mDirectory);
        QString err;
        if(mDirectory.size() == 0) {
            err.append(QStringLiteral("Error: Invalid empty modules path"));
        }
        if(!dir.exists()) {
            err.append(QStringLiteral("Error: Module directory does not exist."));
        }

        if(err.size()) {
            qDebug() << err;
            QMetaObject::invokeMethod(this, "directoryError", Qt::QueuedConnection,  Q_ARG(QString, err));
            exit();
            return;
        }

        QDirIterator itr(dir.path(), {"manifest.json"} , QDir::Files, QDirIterator::Subdirectories);
        while(itr.hasNext()) {
            auto filePath = itr.next();
            QFile manifestFile(filePath);
            if(manifestFile.open(QFile::ReadOnly)){
                auto doc{QJsonDocument::fromJson(manifestFile.readAll())};
                auto manifestObj{doc.object()};
                if(checkValidManifestFile(filePath.left(filePath.lastIndexOf('/')+1), manifestObj)) {
                    mData.append(manifestObj);
                    emit moduleCached(manifestObj);
                }
            }
        }
        exit();
    }

protected:
    QJsonArray   mData;
    QString      mDirectory;

};


class InstallModuleFiles : public QObject
{

};

class CheckInstalledModules : public QObject
{

};

class FetchRepoManifests : public QObject
{

public:
    QString mUrl;

protected:


};

class FetchModuleFiles : public QObject
{

};

class FetchImageFiles : public QThread
{
    Q_OBJECT

signals:
    void lookingForImages(); // notify to clear and show signal
    void imageFound(QJsonObject image);
    void foundImagesFinished(QJsonArray images);

public:
    FetchImageFiles(QString images_directory, QObject* parent = nullptr) : QThread{parent}, mDirectory{images_directory} {}
    void run() override
    {
        QTimer::singleShot(0, this, &FetchImageFiles::startLooking);
        exec(); // Built in event loop now?
        QMetaObject::invokeMethod(this, "foundImagesFinished", Qt::QueuedConnection, Q_ARG(QJsonArray, mImages));
    }
    QJsonArray getImagePaths() { return mImages; }

protected slots:
    void startLooking()
    {
        QMetaObject::invokeMethod(this, &FetchImageFiles::lookingForImages, Qt::QueuedConnection);
        QDir dir(mDirectory);
        qDebug() << "Searching for files in: " << mDirectory;
        QStringList sl;
        QStringList localImages;
        auto supportedFormats{QImageReader::supportedImageFormats()};
        for(const auto & byteArray : qAsConst(supportedFormats)) {
            sl << "*." + byteArray;
        }
        qDebug() << "Supporting Image formats: " << sl;
        QDirIterator itr(dir.path(), sl , QDir::Files, QDirIterator::Subdirectories);
        while(itr.hasNext()) {
            auto filePath = itr.next();
            qDebug() << "file:" << filePath;
            auto fileInfo = itr.fileInfo();
            auto jsonPath = filePath.left(filePath.lastIndexOf(".")+1);
            jsonPath.append("json");
            QFile jsonf(jsonPath);
            bool found{false};
            if(jsonf.open(QFile::ReadOnly)) {
                auto doc{QJsonDocument::fromJson(jsonf.readAll())};
                if(doc.isObject()){
                    auto imageData{doc.object()};
                    if(!imageData.isEmpty()){
                        imageData.insert(SharedKeys::Entry, filePath);
                        qDebug() << filePath;
                        QMetaObject::invokeMethod(this, "imageFound", Qt::QueuedConnection, Q_ARG(QJsonObject, imageData));
                        mImages.append(imageData);
                        found = true;
                    }
                }
            }
            if(!found){
                QJsonObject data;
                data.insert(SharedKeys::Entry, filePath);
                data.insert(SharedKeys::Url, filePath);
                data.insert(SharedKeys::Name, fileInfo.completeBaseName());
                data.insert(SharedKeys::ContentType, "Image");
                QMetaObject::invokeMethod(this, "imageFound", Qt::QueuedConnection, Q_ARG(QJsonObject, data));
                mImages.append(data);
            }
        }
        exit();
    }

protected:
    QString    mDirectory;
    QJsonArray mImages;
};

/************************ Fetch Manifest Files Couroutine *****************************
 * Input: network reply, fileList, retryCount, userAgent
 *
 * Processing:
 *   * For each file in file list
 *      * Use an HTTP GET to fetch the manifest.json in the path
 *         * process the json into a QJsonObject
 *         * If error,
 *            * retry up to retry count
 *            * if still there is an error, report it
 *         * else, put the object into the output vector
 *   * If receive cancel, quit coroutine
 * Output: hadError, manifestFiles
 *************************************************************************************/
class FetchManifestFiles : public QObject
{
    Q_OBJECT

    // (Output) interface to the Main thread
signals:
    void errored(QString error_str,
                 KZPCoroutines::ErrorAction error_type = ErrorAction::RECOVERABLE);

    void progressChanged(int percent);
    void taskStarted(QString task, KZPCoroutines::TaskType type);
    void taskFinished(QString task, KZPCoroutines::TaskType type);
    void manifestFilesReceived(const QVector<QJsonObject>& manifestFiles);

   // (Input) interface


public:
    explicit FetchManifestFiles(QObject* parent = nullptr) : QObject(parent), mTimer{nullptr}, mFetcher{nullptr} {}
    ~FetchManifestFiles()
    {
        cleanUp();
    }
    // Input
    int     retryCount{5};
    QString taskName;
    QByteArray userAgent{USER_AGENT};
    QVector<FileRequest> fileList;

    // Output
    bool hadError{false};

public slots:
    void cancelOperation()
    {

    }
/************************* Processing  ***********************************/
    void run() {
        if(fileList.size() == 0) {
            return;
        }

        mIndex = 0;
        mFetcher = new FileFetcher(this, userAgent);
        mTimer = new QTimer;
        mTimer->setInterval(20000);
        connect(mTimer, &QTimer::timeout,  this, [this](){
            hadError = true;
            emit errored("Timed out after 20 seconds");
            //cleanUp();
        });
        connect(mFetcher, &FileFetcher::fetchFileError, this, [](QJsonObject error){
           qDebug() << "File Fetcher Errored" << error;
        });
        connect(mFetcher, &FileFetcher::fetchingFile, this, [this](QString path){
            emit taskStarted(path, TaskType::FETCH_MANIFEST);
        });
        connect(mFetcher, &FileFetcher::fileReceived, this, [this](QString path, QByteArray data){
            auto doc{QJsonDocument::fromJson(data)};
            if(doc.isObject()) {
                mManifestFiles.append(doc.object());
            }
            emit taskFinished(path, TaskType::FETCH_MANIFEST);
            if(mManifestFiles.size() == fileList.size()) {
                emit manifestFilesReceived(mManifestFiles);
            }
        });
        mFetcher->fetchFiles(fileList);
    }

protected:
    void cleanUp()
    {
        delete mFetcher;
        mFetcher = nullptr;
        delete mTimer;
        mTimer = nullptr;
        mManifestFiles.clear();
    }
    short        mIndex;
    QTimer*      mTimer;
    FileFetcher* mFetcher;
    QVector<QJsonObject> mManifestFiles;
};


}; // end Network Coroutines

Q_DECLARE_METATYPE(KZPCoroutines::TaskType)

#endif // KZP_COROUTINES_H
