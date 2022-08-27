#include "sensor_controller.h"
#include <string_view>
#include <iostream>
#include "sensors-c++/sensors.h"
#include "kzp_keys.h"
#include <QDebug>
#include <QTimer>


void HWSensorController::checkSensors()
{

    // Stop update poll timer
    mPollTimer->stop();

    if(mUpdateList->size() > 0) {
        for(auto iter{mUpdateList->begin()}; iter != mUpdateList->end(); iter++){
            sensors::subfeature f(std::string_view((*iter)->path));
            (*iter)->value = f.read();
        }
        emit sensorUpdate(mUpdateList);
        mPollTimer->start();
    }
}


int subfeatureTypeToValueType(const sensors::subfeature_type t)
{
    using VTYPE = KZPSensors::ValueTypes::ValueType;
    VTYPE ret{int(t)};
    switch(t) {
        case sensors::subfeature_type::average_lowest:
        case sensors::subfeature_type::lowest:
        case sensors::subfeature_type::input_lowest: {
            ret = VTYPE::Lowest;
            break;
        }
        case sensors::subfeature_type::min: {
            ret = VTYPE::Minimum;
            break;
        }
        case sensors::subfeature_type::pulses:
        case sensors::subfeature_type::average:
        case sensors::subfeature_type::input: {
            ret = VTYPE::Value;
            break;
        }
        case sensors::subfeature_type::type:{
            ret = VTYPE::TypeID;
            break;
        }
        case sensors::subfeature_type::crit:
        case sensors::subfeature_type::emergency:
        case sensors::subfeature_type::max: {
            ret = VTYPE::Maximum;
            break;
        }
        case sensors::subfeature_type::input_highest:
        case sensors::subfeature_type::highest:
        case sensors::subfeature_type::average_highest: {
            ret = VTYPE::Highest;
            break;
        }
        default:;
    }

    return int(ret);
}

int featureTypeToValueType(const sensors::feature_type t)
{
    switch(t) {
        case sensors::feature_type::temp: {
            return int(KZPSensors::ValueTypes::ValueType::Temperature);
        }
        case sensors::feature_type::power: {
            return int(KZPSensors::ValueTypes::ValueType::Current);
        }
        case sensors::feature_type::energy: {
            return int(KZPSensors::ValueTypes::ValueType::Wattage);
        }
        case sensors::feature_type::fan:{
            return int(KZPSensors::ValueTypes::ValueType::Speed);
        }
        default:;
    }
    return int(KZPSensors::ValueTypes::ValueType::Value);
}

QString featureTypeToQString(const sensors::feature_type t)
{
    QString ret;
    switch(t) {
        case sensors::feature_type::in: {
            ret = QLatin1String("in");
            break;
        }
        case sensors::feature_type::fan: {
            ret = QLatin1String("Fan");
            break;
        }
        case sensors::feature_type::temp: {
            ret = QLatin1String("Temperature");
            break;
        }
        case sensors::feature_type::power: {
            ret = QLatin1String("Power");
            break;
        }
        case sensors::feature_type::energy: {
            ret = QLatin1String("Energy");
            break;
        }
        case sensors::feature_type::current: {
            ret = QLatin1String("Current");
            break;
        }
        case sensors::feature_type::humidity: {
            ret = QLatin1String("Humidity");
            break;
        }
        case sensors::feature_type::vid: {
            ret = QLatin1String("vid");
            break;
        }
        case sensors::feature_type::intrusion: {
            ret = QLatin1String("intrusion");
            break;
        }
        case sensors::feature_type::beep: {
            ret = QLatin1String("beep");
            break;
        }
        default:ret = QLatin1String("unknown");
    }

    return ret;
}
namespace LinuxSensors{

    constexpr char CPU[] = "cpu";
    constexpr char GPU[] = "gpu";
    constexpr char NVME[] = "nvme";
    constexpr char CPUCORES[] = "coretemp-isa";
    constexpr char MOBO[] = "-isa-";
    constexpr char NVIDIA[] = "nvidia";
    constexpr char RADEON[] = "radeon";
    constexpr char AMD8[] = "k8temp";
    constexpr char AMD10[] = "k10temp";
    constexpr char AMDGPU[] = "amdgpu";
    constexpr char WIFI[] = "wifi";
    KZPSensors::DeviceTypes::Device guessDeviceType(QString name, QString path, QString prefix = QString{})
    {
        if(prefix.size() > 0) {
            if(prefix.contains(NVME, Qt::CaseInsensitive)) {
                return KZPSensors::DeviceTypes::Device::Storage;
            }
            if(prefix.contains(RADEON, Qt::CaseInsensitive)) {
                return KZPSensors::DeviceTypes::Device::GPU;
            }
            if(prefix.contains(NVIDIA, Qt::CaseInsensitive)) {
                return KZPSensors::DeviceTypes::Device::GPU;
            }
            if(prefix.contains(AMDGPU, Qt::CaseInsensitive)) {
                return KZPSensors::DeviceTypes::Device::GPU;
            }
            if(prefix.contains(AMD8, Qt::CaseInsensitive)) {
                return KZPSensors::DeviceTypes::Device::CPU;
            }
            if(prefix.contains(AMD10, Qt::CaseInsensitive)) {
                return KZPSensors::DeviceTypes::Device::CPU;
            }
            if(prefix.contains(WIFI, Qt::CaseInsensitive)) {
                return KZPSensors::DeviceTypes::Device::Network;
            }
        }
        if(name.size() > 0) {
            if(name.startsWith(CPUCORES, Qt::CaseInsensitive)) {
                return KZPSensors::DeviceTypes::Device::CPU;
            }
            if(name.contains(MOBO, Qt::CaseInsensitive)){
                return KZPSensors::DeviceTypes::Device::Motherboard;
            }
            if(name.contains(GPU, Qt::CaseInsensitive)) {
                return KZPSensors::DeviceTypes::Device::GPU;
            }
            if(name.contains(CPU, Qt::CaseInsensitive)) {
                return KZPSensors::DeviceTypes::Device::CPU;
            }
        }
        return KZPSensors::DeviceTypes::Device::Unknown;
    }

};

using DeviceType = KZPSensors::DeviceTypes::Device;
using CPUSensor = KZPSensors::CPU::CPU_Sensor;
using GPUSensor = KZPSensors::GPU::GPU_Sensor;
using ValueType = KZPSensors::ValueTypes::ValueType;
constexpr char CCD[] = "ccd";
constexpr char TDIE[] = "tdie";
constexpr char TCTL[] = "tctl";
constexpr char MEM[] = "mem"; // memory
constexpr char JUNC[] = "junc"; // junction
int  suggestSensorDefaultMap(QString label, int valueType, int subType, int dtype)
{
    if(subType != int(KZPSensors::ValueTypes::ValueType::Value)) {
        return -1;
    }
    switch(KZPSensors::DeviceTypes::Device(dtype)) {
        case DeviceType::CPU:{
            switch(ValueType(valueType)) {
                case ValueType::Temperature: {
                   // CPU Temp, use label to help identify
                   if(label.contains(CCD, Qt::CaseInsensitive)) {
                       //Core Die temp
                       return int(CPUSensor::DieTemp);
                   }
                   if(label.contains(TDIE, Qt::CaseInsensitive)){
                       // Avg Die Temp
                       return int(CPUSensor::AverageTemp);
                   }
                   if(label.contains(TCTL, Qt::CaseInsensitive)){
                       return int(CPUSensor::PackageTemp);
                   }
                   return int(CPUSensor::AverageTemp); // default CPU Temp
                }
                default:;
            }
            break;
        }
        case DeviceType::GPU:{
        switch(ValueType(valueType)) {
            case ValueType::Temperature: {
               if(label.contains(MEM, Qt::CaseInsensitive)) {
                   //Memory Temp
                   return int(GPUSensor::MemoryTemp);
               }
               if(label.contains(JUNC, Qt::CaseInsensitive)){
                   // Junction temp
                   return int(GPUSensor::JunctionTemp);
               }
               return int(GPUSensor::CoreTemp); // default CPU Temp
            }
            default:;
        }
        break;
    }
        default:;
    }

    return -1;
}

void HWSensorController::generateAvailableSensors(QJsonArray& available, QJsonObject settings, bool attach)
{
    Q_UNUSED(attach)
    // get list of available sensors
    auto chips = sensors::get_detected_chips();
    // for chips
    for(const auto& chip: chips) {
        qDebug() << "Found Chip: " << chip.name().data();
        qDebug() << "Path: " << chip.path().data();
        qDebug() << "Prefix: " << chip.prefix().data();
        qDebug() << "Address: " << chip.address();
        qDebug() << "Bus Adapter Name: " << chip.bus().adapter_name().data();
        auto features = chip.features();
        QJsonObject chipObj;
        //auto device{new DeviceDefinition};
        auto prefix {chip.prefix()};
        auto name {chip.name()};
        auto path {chip.path()};
        auto nameStr = QString::fromLatin1(name.data(),name.size());
        auto pathStr = QString::fromLatin1(path.data(),path.size());
        auto prefixStr = QString::fromLatin1(prefix.data(),prefix.size());
        auto  type = LinuxSensors::guessDeviceType(nameStr, pathStr, prefixStr);
        if(name.size() > 0) {
            chipObj.insert(SharedKeys::Name, nameStr);
        }else {
            chipObj.insert(SharedKeys::Name, prefixStr);
        }
        chipObj.insert(SharedKeys::Type, int(type));
        if(features.size() > 0) {
            qDebug() << features.size() << " Features Available";
            QJsonArray sensors;
            for(const auto& feature : features) {
                if(feature.type() == sensors::feature_type::in){
                    continue;
                }
                auto featureName{QString::fromLatin1(feature.name().data(), feature.name().size())};
                auto featureLabel{QString::fromLatin1(feature.label().data(), feature.label().size())};
                qDebug() << "Found Feature: " << featureName;
                qDebug() << "Feature Label: " << featureLabel;
                auto subfeatures = feature.subfeatures();
                qDebug() << "SubFeatures: " << subfeatures.size();
                qDebug() << "Type: " << featureTypeToQString(feature.type());
                QJsonObject sensor;
                auto vtype = featureTypeToValueType(feature.type());

                // set feature level data members
                QJsonArray attributes;
                for(const auto& sub : subfeatures) {
                    if(!((sub.type() == sensors::subfeature_type::crit_hyst) || (sub.type() == sensors::subfeature_type::alarm))) {
                        QJsonObject attr;
                        qDebug() << "Found SubFeature: " << sub.name().data() << " @ [" << sub.number() << "]";
                        auto name{QString::fromLatin1(sub.name().data(), sub.name().size())};
                        auto path{QString::fromLatin1(chip.path().data(), chip.path().size()) + "/" + name};
                        auto stype{subfeatureTypeToValueType(sub.type())};
                        auto watchedSensor{watchingSensor(path)};
                        if(watchedSensor) {
                            AdjustedSensorID id;
                            id.setAdjusted(watchedSensor->sensor);
                            attr.insert(SharedKeys::Sensor, int(id.sensor));
                            attr.insert(SharedKeys::Device, int(id.device));
                        }else {
                            attr.insert(SharedKeys::Sensor, QJsonValue::Null);
                            attr.insert(SharedKeys::Device, QJsonValue::Null);
                        }
                        attr.insert(SharedKeys::Name, name);
                        attr.insert(SharedKeys::Type, vtype);
                        attr.insert(SharedKeys::SubType, stype);
                        attr.insert(SharedKeys::Path, path);
                        attr.insert(SharedKeys::ReadOnly, !sub.writable());
                        try{
                            sensors::subfeature f(std::string_view(path.toStdString()));
                            auto v{f.read()};
                            attr.insert(SharedKeys::Value, v);
                            auto adjustedSensor{-1};
                            AdjustedSensorID id;
                            if(settings.size() <= 1) {
                                // do the automap thing
                                auto sid{suggestSensorDefaultMap(featureLabel, vtype, stype, int(type))};
                                if(sid != -1) {
                                    id.sensor = sid;
                                    adjustedSensor = id.adjusted();
                                }
                            }else if(settings.contains(path)){ // correct and valid mapping found
                                adjustedSensor = settings.value(path).toInt(-1);
                            }
                            if(adjustedSensor != -1) {

                                id.setAdjusted(adjustedSensor);
                                auto sensor{watchingSensorID(adjustedSensor)};
                                while(sensor)
                                {
                                    ++id.device;
                                    sensor = watchingSensorID(id.adjusted());
                                }
                                watchSensor(id.adjusted(), path);
                                attr.insert(SharedKeys::Device, int(id.device));
                                attr.insert(SharedKeys::Sensor, int(id.sensor));
                            }
                            attributes.append(attr);
                        }catch(...) {
                            qDebug() << "Failed to read " << path;
                        }
                    }
                }
                sensor.insert(SharedKeys::Name, featureName);
                sensor.insert(SharedKeys::Label, featureName.compare(featureLabel) == 0 ? QString{}:featureLabel);
                sensor.insert(SharedKeys::Type, vtype);
                sensor.insert(SharedKeys::Attributes, attributes);
                sensors.append(sensor);
            }
            chipObj.insert(SharedKeys::Sensors, sensors);
        }
        available.insert(available.size(),chipObj);
        qDebug() << "\n";
    }
}

void HWSensorController::initialize(QJsonObject settings)
{
    qDebug() << "Received initialize with settings" << settings;

    // Initialize the internal system
    if(!mPollTimer) {
        mPollTimer = new QTimer; 

        if(!mUpdateList) {
            mUpdateList = new UpdateList;
        }
        auto thread{mPollTimer->thread()};
        connect(mPollTimer, &QTimer::timeout, this, &HWSensorController::checkSensors);
        connect(thread, &QThread::finished, this, [this](){
            qDebug() << "Deleting from thread release";
            delete mPollTimer;
            mPollTimer = nullptr;
            delete mUpdateList;
            mUpdateList = nullptr;
        });

        int pollTimer{settings.value(SharedKeys::PollDelay).toInt(-1)};
        if(pollTimer) {
            mPollTimer->setInterval(pollTimer);
        } else {
            mPollTimer->setInterval(3000);
        }
        QJsonArray deviceDefinitions;
        generateAvailableSensors(deviceDefinitions, settings);
        // create sensor map, create devicelist
        emit sensorMapGenerated(deviceDefinitions);
    }

}
void HWSensorController::requestAvailableSensors(QJsonObject settings)
{
    QJsonArray d;
    generateAvailableSensors(d, settings);
    emit sensorsAvailable(d);
}


//void HWSensorController::startMonitoring(int pollDelay)
//{
//    if(mPollTimer == nullptr){
//        mUpdateList = new UpdateList;
//        auto thread{mPollTimer->thread()};
//        connect(mPollTimer, &QTimer::timeout, this, &HWSensorController::checkSensors);
//        connect(thread, &QThread::finished, this, [this](){
//            qDebug() << "Deleting from thread release";
//            if(mPollTimer) {
//                delete mPollTimer;
//                mPollTimer = nullptr;
//            }
//            clearWatching();
//        });
//    }
//    mPollTimer->setInterval(pollDelay);
//    qDebug() << "Started monitoring sensors";
//    QTimer::singleShot(10, this, &HWSensorController::generateSensorMap);
//}



void HWSensorController::stopMonitoring()
{
    if(mPollTimer) {
        QTimer::singleShot(1,mPollTimer, [this](){
            mPollTimer->thread()->exit();
        });
    }
}

void HWSensorController::releaseSensor(AdjustedID id)
{
    auto sensor{watchingSensorID(id)};
    if(!sensor) {
        mUpdateList->removeAll(sensor);
        delete sensor;
        sensor = nullptr;
    }
}

void HWSensorController::writeSensor(AdjustedID id, double value)
{
    auto sensor{watchingSensorID(id)};
    if(!sensor) {
        sensors::subfeature sub(std::string_view{sensor->path});
        if(sub.writable()) {
            sub.write(value);
        }
    }
}

void HWSensorController::watchSensor(AdjustedID id, const QString sensorPath)
{
    if(sensorPath.size() == 0){
        return;
    }
    // check if id is already being watched
    auto sensor{watchingSensorID(id)};
    if(!sensor) {
        auto buffer{new char[sensorPath.size()+1]};
        buffer[sensorPath.size()] = '\0';
        for(int i{0}; i < sensorPath.size(); ++i) {
            buffer[i] = sensorPath.at(i).toLatin1();
        }
        sensors::subfeature sub(std::string_view{buffer});
        if(sub.readable()) {
            sensor = new SensorUpdate{id, sub.read(), buffer};
            mUpdateList->append(sensor);
            if(!mPollTimer->isActive()) {
                mPollTimer->start();
            }
        }
    }
}
