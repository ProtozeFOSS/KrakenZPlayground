#include "system_monitor.h"
#include <QTimer>
#include <QDebug>
#include <QQmlProperty>
#include <QMutableListIterator>
#include <QMetaObject>
#include <QFile>
#include <QVariant>
#include "kzp_keys.h"
#include <QAbstractEventDispatcher>
#ifdef Q_OS_WIN64

#include <map>
using std::map;
#include <lhwm-cpp-wrapper.h>

static constexpr char RELEASE[] = "releaseSensor";
static constexpr char MONITOR[] = "startMonitoring";
static constexpr char WATCH[] = "watchSensor";

struct SensorDefinition{
    ~SensorDefinition(){name.clear();path.clear();type.clear();}
    QString  name;
    QString  type;
    QString  path;
};

struct DeviceDefinition{
    ~DeviceDefinition(){name.clear(); sensors.clear();}
    QString                        name;
    std::vector<SensorDefinition*>  sensors;
};

HWSensorReader::HWSensorReader(QObject* parent)
    : QObject{parent}, mUpdateList{nullptr}, mPollTimer{nullptr}
{

}

HWSensorReader::~HWSensorReader()
{
    mPollTimer = nullptr; // was deleted from thread exit
}




void HWSensorReader::checkSensors()
{
    //qDebug() << "Updating sensors";
    if(mUpdateList && mUpdateList->size() > 0) {
        for(int i{0}; i < mUpdateList->size(); ++i){
            auto sensor{mUpdateList->at(i)};
            sensor->second = LHWM::GetSensorValue(sensor->first);
        }
        emit sensorUpdate(mUpdateList);
    }else {
        mPollTimer->stop();
    }
}


void HWSensorReader::clearWatching()
{
    auto iter{mUpdateList->begin()};
    while(iter != mUpdateList->end()){
        delete [] (*iter)->first;
        delete (*iter);
        (*iter) = nullptr;
        iter = mUpdateList->erase(iter);
    }
    delete mUpdateList;
    mUpdateList = nullptr;
}

void HWSensorReader::generateSensorMap()
{
    // Get the libre HW map, generate a vector of SensorDefinitions
    auto hwMap{LHWM::GetHardwareSensorMap()};
    auto devices{new QList<DeviceDefinition*>};
    for (auto & [deviceName, deviceSensors] : hwMap)
    {
        auto device{new DeviceDefinition};
        device->name = QString::fromStdString(deviceName);
        for(auto const& sensorDesc : deviceSensors)
        {
            auto sdef{new SensorDefinition{QString::fromStdString(std::get<0>(sensorDesc)),
                                          QString::fromStdString(std::get<1>(sensorDesc)),
                                          QString::fromStdString(std::get<2>(sensorDesc))}};
            device->sensors.push_back(sdef);
        }
        devices->append(device);
        deviceSensors.clear();
    }
    hwMap.clear();
    emit sensorMapUpdate(devices);
}



void HWSensorReader::requestSensorMap()
{
    QTimer::singleShot(1, this, &HWSensorReader::generateSensorMap);
}


#endif

void HWSensorReader::startMonitoring(int pollDelay)
{
    if(mPollTimer == nullptr){
        mPollTimer = new QTimer;
        mUpdateList = new UpdateList;
        auto thread{mPollTimer->thread()};
        connect(mPollTimer, &QTimer::timeout, this, &HWSensorReader::checkSensors);
        connect(thread, &QThread::finished, this, [this](){
            qDebug() << "Deleting from thread release";
            if(mPollTimer) {
                delete mPollTimer;
                mPollTimer = nullptr;
            }
            clearWatching();
        });
    }
    mPollTimer->setInterval(pollDelay);
    qDebug() << "Started monitoring sensors";
    QTimer::singleShot(10, this, &HWSensorReader::generateSensorMap);
}

void HWSensorReader::stopMonitoring()
{
    if(mPollTimer) {
        QTimer::singleShot(1,mPollTimer, [this](){
            mPollTimer->thread()->exit();
        });
    }
}

void HWSensorReader::releaseSensor(const QString sensorPath)
{
    QTimer::singleShot(1, mPollTimer, [this, sensorPath](){
        auto iter{mUpdateList->begin()};
        while( iter != mUpdateList->end()){
            if(sensorPath.compare((*iter)->first) == 0) {
                delete [] (*iter)->first;
                (*iter)->first = nullptr;
                delete (*iter);
                (*iter) = nullptr;
                iter = mUpdateList->erase(iter);
            }else {
                ++iter;
            }
        }
    });
}

void HWSensorReader::updateReceived()
{
    QTimer::singleShot(10, mPollTimer, [this](){
        auto aed{QAbstractEventDispatcher::instance()};
        aed->processEvents(QEventLoop::AllEvents);
        mPollTimer->start();
    });
}


void HWSensorReader::watchSensor(const QString sensorPath)
{
    if(sensorPath.size() == 0){
        return;
    }
    bool found{false};
    auto iter{mUpdateList->begin()};
    while( iter != mUpdateList->end()){
        if((found = sensorPath.compare((*iter)->first) == 0)){
            break;
        }
    }
    if(!found) {
        QTimer::singleShot(1, mPollTimer, [this, sensorPath](){
            auto buffer{new char[sensorPath.size()+1]};
            buffer[sensorPath.size()] = '\0';
            for(int i{0}; i < sensorPath.size(); ++i) {
                buffer[i] = sensorPath.at(i).toLatin1();
            }
            mUpdateList->append(new SensorUpdate(buffer, LHWM::GetSensorValue(buffer)));
            mPollTimer->start();
        });
    }
}



/************************ System Monitor ****************************/

SystemMonitor::SystemMonitor(QObject *parent)
    : QObject{parent}, mReader{nullptr}, mReaderThread{this}, mPollDelay{1200}
{
    mReader = new HWSensorReader;
    if(mReader){
        mReader->moveToThread(&mReaderThread);
        connect(mReader, &HWSensorReader::sensorUpdate, this, &SystemMonitor::onSensorUpdate, Qt::BlockingQueuedConnection);
        connect(mReader, &HWSensorReader::sensorMapUpdate, this, &SystemMonitor::onSensorMapUpdate, Qt::BlockingQueuedConnection);
        connect(&mReaderThread, &QThread::started, this, [this](){
           QMetaObject::invokeMethod(mReader, MONITOR, Qt::QueuedConnection, Q_ARG(int, int(mPollDelay)));
        });
        connect(&mReaderThread, &QThread::finished, this, [this](){
            mReader->deleteLater();
            mReader = nullptr;
         });
        mReaderThread.start();
    }
}

void SystemMonitor::aboutToExit()
{
    QMetaObject::invokeMethod(mReader, &HWSensorReader::stopMonitoring, Qt::QueuedConnection);
    QTimer::singleShot(100, this, [this](){
        mReaderThread.exit();
        mReaderThread.wait(10);
        mReaderThread.terminate();
        mReaderThread.wait(10);
    });
}

void SystemMonitor::clearCache()
{
    mSensorMap.clear();
    mDevices.clear();
    mTypes.clear();
    auto keys{mMap.keys()};
    for(const auto& key: qAsConst(keys)) {
        auto mappedSensor{mMap.take(key)};
        auto listeners{mappedSensor.second};
        for(int i{0}; i < listeners->size(); ++i){
            auto mappedProperty{listeners->at(i)};
            delete mappedProperty.second;
            mappedProperty.second = nullptr;
        }
        delete listeners;
        mappedSensor.second = nullptr;
    }
    mMap.clear();

}


SystemMonitor::~SystemMonitor()
{
    if(mReader) {
        mReaderThread.exit();
        mReader->deleteLater();
        mReader = nullptr;
    }
    clearCache();
}

bool SystemMonitor::connectOn(const QJsonObject& sensor, QObject* item, const QString& propertyName)
{
    auto sensorPath{sensor.value(SharedKeys::Path).toString()};
    if(sensorPath.size() > 0) {
        return connectOn(sensorPath, item, propertyName);
    }
    return false;
}

bool SystemMonitor::connectOn(const QString& sensorPath, QObject* item, const QString& propertyName)
{
    if(item && mSensorMap.contains(sensorPath)) { // item and sensor both exists
        auto propertyToBind = item->property(propertyName.toLatin1().data());
        if(propertyToBind.isValid()) {
            auto propertyValueType = propertyToBind.type();
            switch(propertyValueType) { // all values received are floats, check for valid conversion types
                case QVariant::String:
                case QVariant::Double:
                case QVariant::Int:
                case QVariant::LongLong:
                {
                    QList<MappedProperty>* list{nullptr};
                    float value{0};
                    if(mMap.contains(sensorPath)) {
                        auto mapping{mMap.take(sensorPath)};
                        value = mapping.first;
                        list = mapping.second;
                    }else {
                        list = new QList<MappedProperty>;
                    }
                    if(list->size() == 0) {
                        QMetaObject::invokeMethod(mReader, WATCH, Qt::QueuedConnection, Q_ARG(QString, sensorPath));
                    }
                    connect(item, &QObject::destroyed, this, &SystemMonitor::onItemDestroyed, Qt::UniqueConnection);
                    list->append(MappedProperty(item, new QQmlProperty(item, propertyName)));
                    mMap.insert(sensorPath, MappedSensor(value, list));
                    return true;
                }
                default:;
            }
        }
    }
    return false;
}

float SystemMonitor::currentValue(const QString& sensorPath)
{
    float f{0.0};
    if(mMap.contains(sensorPath)) {
        f = mMap.value(sensorPath).first;
    }
    return f;
}

void SystemMonitor::onSensorMapUpdate(void* deviceListPtr)
{
    auto devices{static_cast<QList<DeviceDefinition*>*>(deviceListPtr)};
    if(devices) {
        mSensorMap.clear();
        mDevices.clear();
        mTypes.clear();
        quint32 deviceID{0};
        for(auto& device : *devices) {
            mDevices.append(device->name);
            auto sensors{device->sensors};
            for(auto& sensor : sensors) {
                auto typeID{mTypes.indexOf(sensor->type)};
                if(typeID < 0 ) {
                    mTypes.append(sensor->type);
                    typeID = mTypes.size()-1;
                }
                mSensorMap.insert(sensor->path, SensorData{sensor->name, deviceID, typeID});
                delete sensor;
                sensor = nullptr;
            }
            sensors.clear();
            ++deviceID;
            delete device;
            device = nullptr;
        }        
        delete devices;
        devices = nullptr;
        auto keys{mMap.keys()};
        for(const auto& key: qAsConst(keys)) {
            if(!mSensorMap.contains(key)) {
                auto mappedSensor{mMap.take(key)};
                auto listeners{mappedSensor.second};
                for(int i{0}; i < listeners->size(); ++i){
                    auto mappedProperty{listeners->at(i)};
                    delete mappedProperty.second;
                    mappedProperty.second = nullptr;
                }
                delete listeners;
                mappedSensor.second = nullptr;
                qDebug() << "Sensor " << key << "is no longer available.";
                QMetaObject::invokeMethod(mReader, RELEASE, Qt::QueuedConnection, Q_ARG(QString, key));
            }
        }
        emit sensorsReady();
    }
}

void SystemMonitor::onSensorUpdate(void* updates)
{
    auto updateList{static_cast<UpdateList*>(updates)};
    if(updateList) {
        for(int i{0}; i < updateList->size(); ++i){
            auto sensorUpdate{updateList->at(i)};
            auto sensor{mMap.find(sensorUpdate->first)};
            if(sensor != mMap.end()) {
                if(sensor->first != sensorUpdate->second) {
                    sensor->first = sensorUpdate->second;
                    for(const auto& property : qAsConst(*(sensor->second))){
                        property.second->write(sensor->first);
                    }
                }
            }
        }
    }

    auto aed{QAbstractEventDispatcher::instance()};
    aed->processEvents(QEventLoop::AllEvents);
    QMetaObject::invokeMethod(mReader, &HWSensorReader::updateReceived);
}

bool SystemMonitor::releaseAll(QObject* item)
{
    bool released{false};
    if(item) {
        QList<QString> removeList;
        for (auto iter = mMap.begin(); iter != mMap.end(); ++iter) {
            auto mappedSensor{iter.value()};
            auto key{iter.key()};
            QMutableListIterator i(*mappedSensor.second);
            while(i.hasNext())
            {
                auto property{i.next()};
                if(property.first == item){
                    delete property.second;
                    property.second = nullptr;
                    i.remove();
                }
            }
            if(mappedSensor.second->size() == 0) {
                QMetaObject::invokeMethod(mReader, RELEASE, Qt::QueuedConnection, Q_ARG(QString, key));
                delete mappedSensor.second;
                mappedSensor.second = nullptr;
                removeList.append(key);
            }
        }
        for(const auto& key : removeList) {
            mMap.remove(key);
        }
    }
    return released;
}

void SystemMonitor::onItemDestroyed()
{
    auto obj{sender()};
    releaseAll(obj);
}

bool SystemMonitor::sensorExists(const QString& sensorPath)
{
    if(sensorPath.isEmpty()) {
        return false;
    }
    return mSensorMap.contains(sensorPath);
}

QJsonArray SystemMonitor::sensorsByType(const QString& type, const QString& deviceFilter)
{
    QJsonArray sensors;
    auto paths{mSensorMap.keys()};
    for(const auto& path : paths) {
        auto sensorData{mSensorMap.value(path)};
        auto device{mDevices.at(std::get<1>(sensorData))};
        auto sensorType{mTypes.at(std::get<2>(sensorData))};
        if(sensorType.compare(type) == 0 && (deviceFilter.size() == 0 || path.contains(deviceFilter, Qt::CaseInsensitive))) {
            QJsonObject obj;
            obj.insert(SharedKeys::Device, device);
            obj.insert(SharedKeys::Name, std::get<0>(sensorData));
            obj.insert(SharedKeys::Type, sensorType);
            obj.insert(SharedKeys::Path, path);
            sensors.append(obj);
        }
    }
    return sensors;
}

