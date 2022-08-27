#include "kzp_keys.h"
#include <QAbstractEventDispatcher>
#include "system_monitor.h"
#include "sensor_controller.h"
#include "sensor_monitor.h"
#include <QQmlProperty>
#include <QTimer>

SystemMonitor::SystemMonitor(QObject *parent)
    : QObject{parent}, mSensorController{nullptr}, mSensorThread{this}, mAttachedSensors{nullptr}, mDeviceList{nullptr},  mPollDelay{1200}
{
    mSensorController = new HWSensorController;
    if(mSensorController){
        mSensorController->moveToThread(&mSensorThread);
        connect(mSensorController, &HWSensorController::sensorUpdate, this, &SystemMonitor::onSensorUpdate, Qt::BlockingQueuedConnection);
        connect(mSensorController, &HWSensorController::sensorMapGenerated, this, &SystemMonitor::onSensorMapUpdate);
        connect(mSensorController, &HWSensorController::sensorsAvailable, this, &SystemMonitor::receivedDevices, Qt::BlockingQueuedConnection);

    }
}

void SystemMonitor::aboutToExit()
{
    QMetaObject::invokeMethod(mSensorController, &HWSensorController::stopMonitoring, Qt::QueuedConnection);
    QTimer::singleShot(100, this, [this](){
        mSensorThread.exit();
    });
}

void SystemMonitor::clearCache()
{
    clearAttachedMap();
    clearDeviceList();
}


SystemMonitor::~SystemMonitor()
{
    if(mSensorController) {
        mSensorThread.exit();
        mSensorController->deleteLater();
        mSensorController = nullptr;
    }
    clearCache();
}

//bool SystemMonitor::connectOn(const QJsonObject& sensor, QObject* item, const QString& propertyName)
//{
//    auto sensorPath{sensor.value(SharedKeys::Path).toString()};
//    if(sensorPath.size() > 0) {
//        return connectOn(sensorPath, item, propertyName);
//    }
//    return false;
//}

//bool SystemMonitor::connectOn(const QString& sensorPath, QObject* item, const QString& propertyName)
//{
////    if(item && mSensorMap.contains(sensorPath)) { // item and sensor both exists
////        auto propertyToBind = item->property(propertyName.toLatin1().data());
////        if(propertyToBind.isValid()) {
////            auto propertyValueType = propertyToBind.type();
////            switch(propertyValueType) { // all values received are stored as doubles, check for valid conversion types
////                case QVariant::String:
////                case QVariant::Double:
////                case QVariant::Int:
////                case QVariant::LongLong:
////                {
////                    QList<MappedProperty>* list{nullptr};
////                    double value{0};
////                    if(mMap.contains(sensorPath)) {
////                        auto mapping{mMap.take(sensorPath)};
////                        value = mapping.first;
////                        list = mapping.second;
////                    }else {
////                        list = new QList<MappedProperty>;
////                    }
////                    if(list->size() == 0) {
////                        QMetaObject::invokeMethod(mReader, WATCH, Qt::QueuedConnection, Q_ARG(QString, sensorPath));
////                    }
////                    connect(item, &QObject::destroyed, this, &SystemMonitor::onItemDestroyed, Qt::UniqueConnection);
////                    list->append(MappedProperty(item, new QQmlProperty(item, propertyName)));
////                    mMap.insert(sensorPath, MappedSensor(value, list));
////                    return true;
////                }
////                default:;
////            }
////        }
////    }
//    return false;
//}

//double SystemMonitor::currentValue(const QString& sensorPath)
//{
//    double f{0.0};
//    if(mMap.contains(sensorPath)) {
//        f = mMap.value(sensorPath).first;
//    }
//    return f;
//}


//void SystemMonitor::requestAvailableDevices()
//{
//    if(mSensorController){
//         QMetaObject::invokeMethod(mSensorController, &HWSensorController::requestAvailableSensors);
//    }
//    // fill the array based on devices
//    // iterate through the sensorMap and attempt to make complete map
////    auto paths{mSensorMap.keys()};
////    for(const auto& device : qAsConst(mDeviceList)) {
////        QJsonObject obj;
//////        auto sensorData{mSensorMap.value(path)};
//////        auto device{mDevices.at(std::get<1>(sensorData))};
//////        auto sensorType{mTypes.at(std::get<2>(sensorData))};
////        obj.insert(SharedKeys::Device, device->name);
////        QJsonArray sensor_data;
////        for(const auto& sensor : device->sensors) {
////            QJsonObject sensor_obj;
////            sensor_obj.insert(SharedKeys::Name, sensor->name);
////            sensor_obj.insert(SharedKeys::Type, sensor->type);
////            sensor_obj.insert(SharedKeys::Path, sensor->path);
////            if(mMap.contains(sensor->path)) {
////                //auto mapping = mMap.value(sensor->path);
////                //sensor_obj.insert(SharedKeys::FilePath, mapping.first);
////            }else {

////            }
////            //sensor_obj.insert(SharedKeys::Label, sensor->label);
////            sensor_data.append(sensor_obj);
////        }
////        obj.insert(SharedKeys::Sensors, sensor_data);
////        ret.append(obj);
////    }
//}

void SystemMonitor::receivedDevices(QJsonArray deviceArray)
{
    emit availableDevices(deviceArray);
}

void SystemMonitor::clearAttachedMap()
{
    if(mAttachedSensors) {
        delete mAttachedSensors;
        mAttachedSensors = nullptr;
    }
}

void SystemMonitor::clearDeviceList()
{
    if(mDeviceList) {
        auto device{mDeviceList->begin()};
        while( device != mDeviceList->end()){
            auto sensor{(*device)->sensors.begin()};
            while( sensor != (*device)->sensors.end()){
                delete (*sensor);
                ++sensor;
            }
            delete (*device);
            ++device;
        }
        delete mDeviceList;
        mDeviceList = nullptr;
    }
}

void SystemMonitor::onSensorMapUpdate(QJsonArray deviceArray)
{
    // Paths will be different per system, per OS, but its always
    // Some/Path/Some/Sensor ->(maps to) Value
    // Default Mapping is generated through some simple introspection of the data
    // this is done at the OS level to help abstract the difference in paths
    // pathMap will be used to verify "Settings Mappeded sensors" are valid

    clearAttachedMap();
    clearDeviceList();
    mDeviceList = new QList<DeviceDefinition*>;
    mAttachedSensors = new SensorMap;
    QJsonObject settings;
    // create device list
    // create mapping list
    for(int index{0}; index < deviceArray.size(); ++index) {
        auto deviceValue{deviceArray.at(index)}; // device name, sensors
        auto device{deviceValue.toObject()};
        auto name = device.value(SharedKeys::Name).toString();
        auto type = device.value(SharedKeys::Type).toInt(-1);
        auto deviceDef = new DeviceDefinition;
        if(type > -1) {
            deviceDef->type = KZPSensors::DeviceTypes::Device(type);
        }
        deviceDef->name = name;
        QString sensor_path;

        auto sensorsValue = device.value(SharedKeys::Sensors); // array of sensor objects
        auto sensors = sensorsValue.toArray();
        for(int s{0}; s < sensors.size(); s++) {
            auto sensorValue = sensors.at(s);
            auto sensor = sensorValue.toObject(); // sensor object Name, Label, Type
            auto attributesValue = sensor.value(SharedKeys::Attributes);
            auto attributes = attributesValue.toArray();
            int minId{int(KZPSensors::CPU::CPU_Sensor::DieTemp)};
            for(int a{0}; a < attributes.size(); a++) {
                // create SensorDefinition
                auto attributeValue = attributes.at(a);
                auto attribute = attributeValue.toObject();
                auto type = attribute.value(SharedKeys::Type).toInt(-1);
                auto sensorId = attribute.value(SharedKeys::Sensor).toInt(-1);
                auto device = attribute.value(SharedKeys::Device).toInt(-1);
                auto path = attribute.value(SharedKeys::Path).toString(); // sensor path
                auto sensorDef = new SensorDefinition("", type, path);
                if(sensorId >= minId){
                    // create SensorMapping
                    AdjustedSensorID id{sensorId, device};
                    mAttachedSensors->insert(id.adjusted(), sensorDef);
                    settings.insert(path, id.adjusted());
                    qDebug() << "Created Mapped sensor(" << sensorId << ") on device[" << device << "] @ " << path;
                    sensorDef->setValue(attribute.value(SharedKeys::Value).toDouble(0));
                }
                deviceDef->sensors.append(sensorDef);
            }
        }
        mDeviceList->append(deviceDef);
    }
    mSettings = settings;
    emit sensorsReady();
    emit availableDevices(deviceArray);
}

void SystemMonitor::queryDevicesAvailable()
{    
    QMetaObject::invokeMethod(mSensorController, KZP::REQUEST, Qt::QueuedConnection, Q_ARG(QJsonObject, mSettings));
}

void SystemMonitor::onSensorUpdate(void* updates)
{
    auto updateList{static_cast<UpdateList*>(updates)};
    if(mAttachedSensors && updateList) {
        for(auto iter{updateList->cbegin()}; iter != updateList->end(); ++iter){
            auto sensor{mAttachedSensors->value((*iter)->sensor, nullptr)};
            if(sensor) {
                sensor->setValue((*iter)->value);
            }
        }
    }
//    auto aed{QAbstractEventDispatcher::instance()};
//    aed->processEvents(QEventLoop::AllEvents);
//    QMetaObject::invokeMethod(mSensorController, &HWSensorController::updateReceived);
}

void SystemMonitor::releaseAll()
{
    // release all existing connections
    if(!mAttachedSensors) {
        return;
    }
    auto keys{mAttachedSensors->keys()};
    for(const auto& key :  qAsConst(keys)){
        auto sensor = mAttachedSensors->value(key, nullptr);
        if(sensor) {
            sensor->disconnect("2valueChanged(double)");
            QMetaObject::invokeMethod(mSensorController, KZP::RELEASE, Qt::QueuedConnection, Q_ARG(int, key));
            mAttachedSensors->take(key);
        }
    }
}




bool SystemMonitor::sensorExists(int sensor, int device)
{
    AdjustedSensorID id{sensor, device};

    if(mAttachedSensors->contains(id.adjusted())) {
        return true;
    }
    return false;
}

// NEW Interface

// load mapping from json settings
// save mapping to json settings
// Add and remove from mappings

bool SystemMonitor::isAttached(int sensorID, int device)
{
    AdjustedSensorID id{sensorID, device};
    auto adjustedId{id.adjusted()};
    if(mAttachedSensors->contains(adjustedId)){
        return (mAttachedSensors->value(adjustedId)->mPath.size() > 0);
    }
    return false;
}

bool SystemMonitor::connectOn(int sensorID, int device, QObject& monitor)
{
    if((mAttachedSensors == nullptr) || (device < 0) || (sensorID < int(KZPSensors::CPU::CPU_Sensor::DieTemp))) {
        return false;
    }
    bool success{false};
    AdjustedSensorID id{sensorID, device};
    auto adjustedId{id.adjusted()};
    if(mAttachedSensors->contains(adjustedId)) {
        auto sensor = mAttachedSensors->value(adjustedId);
        if(sensor->mPath.size() > 0 ) { // connect if the sensor is actually attached
            connect(sensor, "2valueChanged(double)", &monitor, "1valueUpdated(double)");
            emit sensor->valueChanged(sensor->mValue);
            success = true;
        }
    }
    return success;
}


void SystemMonitor::onUpdate(int sensorID, double value)
{
    if(!mAttachedSensors->contains(sensorID)){
        return;
    }
    auto sensor = mAttachedSensors->value(sensorID);
    sensor->setValue(value);
}

double SystemMonitor::readSensor(int sensorID, int device, bool * valid)
{
    double ret{0.0};
    bool v{false};

    AdjustedSensorID id{sensorID, device};
    auto adjustedId{id.adjusted()};
    if(mAttachedSensors->contains(adjustedId)) {
        auto sensor{mAttachedSensors->value(adjustedId)};
        ret = sensor->mValue;
        v = true;
    }
    if(valid != nullptr) {
        *valid = v;
    }
    return ret;
}

QJsonObject SystemMonitor::jsonSettings()
{
    QJsonObject data;
    auto keys = mAttachedSensors->keys();
    for(const auto & id: keys) {
        // insert the attachment
        // path : id
        auto attachData = mAttachedSensors->value(id);
        data.insert(attachData->path(), id);
    }
    return data;
}

void SystemMonitor::initializeSensors(QJsonObject sensorMap) // apply previous mappings
{
    if(mSensorController) {
        connect(&mSensorThread, &QThread::started, this, [this,sensorMap](){
            QMetaObject::invokeMethod(mSensorController, KZP::INIT, Qt::QueuedConnection, Q_ARG(QJsonObject, sensorMap));
        });
        connect(&mSensorThread, &QThread::finished, this, [this](){
            mSensorController->deleteLater();
            mSensorController = nullptr;
         });
        mSensorThread.start();
    }

}

bool SystemMonitor::releaseConnection(int sensorID, int device, QObject& monitor)
{
    bool released{false};
    AdjustedSensorID id{sensorID, device};
    auto adjustedId{id.adjusted()};
    if( mAttachedSensors->contains(adjustedId)) {
        auto sensor{mAttachedSensors->value(adjustedId)};
        sensor->disconnect(&monitor);
    }
    return released;
}

void SystemMonitor::writeToSensor(int sensorID, int device, double value, bool* written)
{
    AdjustedSensorID id{sensorID, device};
    auto adjustedId{id.adjusted()};
    if(!mAttachedSensors->contains(adjustedId)){
        return;
    }
    auto sensor{mAttachedSensors->value(adjustedId)};
    if(sensor->canWrite()){
        // send write method to the HWSensorController
        QMetaObject::invokeMethod(mSensorController, KZP::WRITE , Qt::QueuedConnection, Q_ARG(int, adjustedId), Q_ARG(double, value));
    }
    if(written != nullptr) {
        (*written) = sensor->canWrite();
    }
}

// Static Convenience methods
bool SystemMonitor::isValidSensor(int sensorID)
{
    return (sensorID <= int(CPU_Sensor::WorkLoad) && sensorID >= int(CPU_Sensor::DieTemp)) | (sensorID <= int(GPU_Sensor::WorkLoad) && sensorID >= int(GPU_Sensor::CoreTemp));
}

QJsonArray SystemMonitor::sensorsAvailable()
{
    return {
        QJsonObject{{Name, CPUDieTemp}, {Value, int(CPU_Sensor::DieTemp)}},
        QJsonObject{{Name, CPUPackageTemp}, {Value, int(CPU_Sensor::PackageTemp)}},
        QJsonObject{{Name, CPUAverageTemp}, {Value, int(CPU_Sensor::AverageTemp)}},
        QJsonObject{{Name, CPUWorkLoad}, {Value, int(CPU_Sensor::WorkLoad)}},
        QJsonObject{{Name, CPUWorkLoadTotal}, {Value, int(CPU_Sensor::WorkLoadTotal)}},
        QJsonObject{{Name, CPUFrequency}, {Value, int(CPU_Sensor::Frequency)}},
        QJsonObject{{Name, GPUCoreTemp}, {Value, int(GPU_Sensor::CoreTemp)}},
        QJsonObject{{Name, GPUJunctionTemp}, {Value, int(GPU_Sensor::JunctionTemp)}},
        QJsonObject{{Name, GPUMemoryTemp}, {Value, int(GPU_Sensor::MemoryTemp)}},
        QJsonObject{{Name, GPUMemoryUsed}, {Value, int(GPU_Sensor::MemoryUsed)}},
        QJsonObject{{Name, GPUWorkLoad}, {Value, int(GPU_Sensor::WorkLoad)}},
        QJsonObject{{Name, GPUMemorySpeed}, {Value, int(GPU_Sensor::MemorySpeed)}},
        QJsonObject{{Name, GPUFanSpeed}, {Value, int(GPU_Sensor::FanSpeed)}},
        QJsonObject{{Name, GPUVoltage}, {Value, int(GPU_Sensor::Voltage)}},
        QJsonObject{{Name, GPUWattage}, {Value, int(GPU_Sensor::Wattage)}},
    };
}
QJsonArray SystemMonitor::sensorValueTypes()
{
    return {
        QJsonObject{{Name, Temperature}, {Value, int(ValueType::Temperature)}},
        QJsonObject{{Name, Utilization}, {Value, int(ValueType::Utilization)}},
        QJsonObject{{Name, Speed}, {Value, int(ValueType::Speed)}},
        QJsonObject{{Name, DataSize}, {Value, int(ValueType::DataSize)}},
        QJsonObject{{Name, Frequency}, {Value, int(ValueType::Frequency)}},
        QJsonObject{{Name, Voltage}, {Value, int(ValueType::Voltage)}},
        QJsonObject{{Name, Wattage}, {Value, int(ValueType::Wattage)}},
        QJsonObject{{Name, Current}, {Value, int(ValueType::Current)}},
        QJsonObject{{Name, Minimum}, {Value, int(ValueType::Minimum)}},
        QJsonObject{{Name, Maximum}, {Value, int(ValueType::Maximum)}},
        QJsonObject{{Name, Value}, {Value, int(ValueType::Value)}},
        QJsonObject{{Name, Highest}, {Value, int(ValueType::Highest)}},
        QJsonObject{{Name, Lowest}, {Value, int(ValueType::Lowest)}},
        QJsonObject{{Name, TypeID}, {Value, int(ValueType::TypeID)}},
        QJsonObject{{Name, Data}, {Value, int(ValueType::Data)}},
        QJsonObject{{Name, DataRate}, {Value, int(ValueType::DataRate)}}
    };
}


// TODO Make suffix work

QString SystemMonitor::sensorIconSource(int type)
{
    using VTypes = KZPSensors::ValueTypes::ValueType;
    using CPU = KZPSensors::CPU::CPU_Sensor;
    using GPU = KZPSensors::GPU::GPU_Sensor;
    QString data;
    switch(type) {
        case int(CPU::DieTemp):
        case int(CPU::PackageTemp):
        case int(CPU::AverageTemp):
        case int(CPU::WorkLoad):
        case int(CPU::WorkLoadTotal):
        case int(CPU::Frequency):{
            data = QLatin1String{"qrc:/images/cpu.png"};
            break;
        }

        case int(GPU::CoreTemp):
        case int(GPU::JunctionTemp):
        case int(GPU::MemoryTemp):
        case int(GPU::MemoryUsed):
        case int(GPU::WorkLoad):
        case int(GPU::MemorySpeed):
        case int(GPU::FanSpeed):
        case int(GPU::ClockSpeed):
        case int(GPU::Frequency):
        case int(GPU::Voltage):
        case int(GPU::Wattage):{
            data = QLatin1String{"qrc:/images/gpu.png"};
            break;
        }
        case int(VTypes::Temperature): {
            data = QLatin1String{"qrc:/images/high-temperature.png"};
            break;
        }
        case int(VTypes::Speed): {
            data = QLatin1String{"qrc:/images/fan.png"};
            break;
        }
        case int(VTypes::Value): {
            data = QLatin1String{"qrc:/images/factor.png"};
            break;
        }
        case int(VTypes::Current):
        case int(VTypes::Voltage):
        case int(VTypes::Wattage): {
            data = QLatin1String{"qrc:/images/electric-current.png"};
            break;
        }
        case int(VTypes::Frequency): {
            data = QLatin1String{"qrc:/images/frequency.png"};
            break;
        }
        case int(VTypes::Utilization): {
            data = QLatin1String{"qrc:/images/performance.png"};
            break;
        }
        default:;
    }
    return data;
}

QString SystemMonitor::sensorIDToString(int type)
{
    QString data;
    switch(type) {
        // Add Devices
        case int(CPU_Sensor::DieTemp): {
            data = CPUDieTemp;
            break;
        }
        case int(CPU_Sensor::PackageTemp): {
            data = CPUPackageTemp;
            break;
        }
        case int(CPU_Sensor::AverageTemp): {
            data = CPUAverageTemp;
            break;
        }
        case int(CPU_Sensor::WorkLoad): {
            data = CPUWorkLoad;
            break;
        }
        case int(CPU_Sensor::WorkLoadTotal): {
            data = CPUWorkLoadTotal;
            break;
        }
        case int(CPU_Sensor::Frequency): {
            data = CPUFrequency;
            break;
        }
        case int(GPU_Sensor::CoreTemp): {
            data = GPUCoreTemp;
            break;
        }
        case int(GPU_Sensor::JunctionTemp): {
            data = GPUJunctionTemp;
            break;
        }
        case int(GPU_Sensor::MemoryTemp): {
            data = GPUMemoryTemp;
            break;
        }
        case int(GPU_Sensor::MemoryUsed): {
            data = GPUMemoryUsed;
            break;
        }
        case int(GPU_Sensor::WorkLoad): {
            data = GPUWorkLoad;
            break;
        }
        case int(GPU_Sensor::MemorySpeed): {
            data = GPUMemorySpeed;
            break;
        }
        case int(GPU_Sensor::FanSpeed): {
            data = GPUFanSpeed;
            break;
        }
        case int(GPU_Sensor::ClockSpeed): {
            data = GPUClockSpeed;
            break;
        }
        case int(GPU_Sensor::Frequency): {
            data = GPUFrequency;
            break;
        }
        case int(GPU_Sensor::Voltage): {
            data = GPUVoltage;
            break;
        }
        case int(GPU_Sensor::Wattage): {
            data = GPUWattage;
            break;
        }
        // TODO: Add Sensor Types and Value Types

        default: data = (QString{"Unknown["} + QString::number(type) + QString{"]"});
    }
    return data;
}


QString SystemMonitor::sensorTypeToString(int type)
{
    QString ret;

    return ret;
}

