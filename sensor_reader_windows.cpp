#include "sensor_controller.h"
#include <QTimer>
#include <QDebug>
#include <QMetaObject>
#include <QFile>
#include <QVariant>
#include "kzp_keys.h"
#include <QJsonDocument>

#include <map>
using std::map;

#include <lhwm-cpp-wrapper.h>


void HWSensorController::checkSensors()
{
    mPollTimer->stop();
    if(mUpdateList->size() > 0) {
        for(auto iter{mUpdateList->begin()}; iter != mUpdateList->end(); iter++){
            (*iter)->value = LHWM::GetSensorValue((*iter)->path);
        }
        emit sensorUpdate(mUpdateList);
        mPollTimer->start();
    }
}

using DeviceType = KZPSensors::DeviceTypes::Device;
using CPUSensor = KZPSensors::CPU::CPU_Sensor;
using GPUSensor = KZPSensors::GPU::GPU_Sensor;
using ValueType = KZPSensors::ValueTypes::ValueType;

namespace LHWMTypes {
    constexpr char Temp[] = "Temperature";
    constexpr char Load[] = "Load";
    constexpr char Clock[] = "Clock";
    constexpr char Power[] = "Power";
    constexpr char Core[] = "Core";
    constexpr char Junction[] = "Hot";
    constexpr char Memory[] = "Mem";
    constexpr char Factor[] = "Factor";
    constexpr char Voltage[] = "Voltage";
    constexpr char Throughput[] = "Throughput";
    constexpr char SData[] = "SmallData";
    constexpr char Data[] = "Data";
    constexpr char Control[] = "Control";
    constexpr char Fan[] = "Fan";
    constexpr char Package[] = "Package";
    constexpr char Cpu[] = "cpu";
    constexpr char Gpu[] = "gpu";
    constexpr char Mobo[] = "lpc";
    constexpr char Net[] = "nic";
    constexpr char Ram[] = "ram";
    constexpr char NVME[] = "nvme";
    constexpr char Total[] = "total";
    constexpr char Average[] = "average";
    constexpr char Max[] = "max";
    constexpr char CCD[] = "ccd";
    constexpr char TCTL[] = "tctl";
    constexpr char TDIE[] = "tdie";

};
int lhwmTypeToValueType(QString lhwmType)
{
    int ret{-1};
    if(lhwmType.compare(LHWMTypes::Temp) == 0) {
        return int(ValueType::Temperature);
    }
    if(lhwmType.compare(LHWMTypes::Load) == 0) {
        return int(ValueType::Utilization);
    }
    if(lhwmType.compare(LHWMTypes::Clock) == 0) {
        return int(ValueType::Frequency);
    }
    if(lhwmType.compare(LHWMTypes::Voltage) == 0) {
        return int(ValueType::Voltage);
    }
    if(lhwmType.compare(LHWMTypes::Factor) == 0) {
        return int(ValueType::Value);
    }
    if(lhwmType.compare(LHWMTypes::Throughput) == 0) {
        return int(ValueType::DataRate);
    }
    if(lhwmType.compare(LHWMTypes::Data) == 0) {
        return int(ValueType::Data);
    }
    if(lhwmType.compare(LHWMTypes::SData) == 0) {
        return int(ValueType::Data);
    }
    if(lhwmType.compare(LHWMTypes::Fan) == 0) {
        return int(ValueType::Speed);
    }
    if(lhwmType.compare(LHWMTypes::Power) == 0) {
        return int(ValueType::Wattage);
    }
    qDebug() << "Did not map " << lhwmType << "!";

    return ret;
}
int  guessDeviceType(QString path)
{
    if(path.contains(LHWMTypes::Cpu,Qt::CaseInsensitive)) {
        return int(DeviceType::CPU);
    }
    if(path.contains(LHWMTypes::Gpu, Qt::CaseInsensitive)) {
        return int(DeviceType::GPU);
    }
    if(path.contains(LHWMTypes::Mobo, Qt::CaseInsensitive)) {
        return int(DeviceType::Motherboard);
    }
    if(path.contains(LHWMTypes::Net, Qt::CaseInsensitive)) {
        return int(DeviceType::Network);
    }
    if(path.contains(LHWMTypes::Ram, Qt::CaseInsensitive)) {
        return int(DeviceType::Memory);
    }
    if(path.contains(LHWMTypes::NVME, Qt::CaseInsensitive)) {
        return int(DeviceType::Storage);
    }
    return -1;
}

int  suggestSensorDefaultMap(QString type, QString label, QString path, int valueType, int dtype)
{
    int value{-1};
    switch(DeviceType(dtype)) {
        case DeviceType::CPU:{
            switch(ValueType(valueType)) {
                case ValueType::Utilization: {
                    if(label.contains(LHWMTypes::Total, Qt::CaseInsensitive)) {
                        value = int(CPUSensor::WorkLoadTotal);
                    }
                    break;
                }
                case ValueType::Temperature: {
                   // CPU Temp, use label to help identify
                   if(label.contains(LHWMTypes::Average, Qt::CaseInsensitive)) {
                        value = int(CPUSensor::AverageTemp);
                   } else if(label.contains(LHWMTypes::Max, Qt::CaseInsensitive)) {
                       return AdjustedSensorID::adjusted(int(KZPSensors::ValueTypes::ValueType::Maximum));
                   } else if(label.contains(LHWMTypes::CCD, Qt::CaseInsensitive)) {
                       //Core Die temp
                       value = int(CPUSensor::DieTemp);
                   } else if(label.contains(LHWMTypes::TCTL, Qt::CaseInsensitive)){
                       value = int(CPUSensor::PackageTemp);
                   } else if(label.contains(LHWMTypes::TDIE, Qt::CaseInsensitive)){
                       // Avg Die Temp
                       value = int(CPUSensor::AverageTemp);
                   } else {
                       value = int(CPUSensor::PackageTemp); // default CPU Temp
                   }
                   break;
                }
                default:;
            }
            break;
        }
        case DeviceType::GPU:{
            switch(ValueType(valueType)) {
                case ValueType::Temperature: {
                   if(label.contains(LHWMTypes::Core, Qt::CaseInsensitive)) {
                       //Memory Temp
                       value = int(GPUSensor::CoreTemp);
                   } else if(label.contains(LHWMTypes::Junction, Qt::CaseInsensitive)){
                      // Junction temp
                       value = int(GPUSensor::JunctionTemp);
                   } else {
                       value = int(GPUSensor::CoreTemp); // default CPU Temp
                   }
                   break;
                }
                case ValueType::Frequency: {
                    if(label.contains(LHWMTypes::Memory, Qt::CaseInsensitive)){
                        value = int(GPUSensor::MemorySpeed);
                    } else {
                        value = int(GPUSensor::ClockSpeed); // default clock speed
                    }
                    break;
                }
                case ValueType::Utilization: {
                    if(label.contains(LHWMTypes::Memory, Qt::CaseInsensitive)) {
                        value = int(GPUSensor::MemoryUsed);
                    } else {
                        value = int(GPUSensor::WorkLoad);
                    }
                    break;
                }
                case ValueType::Speed: {
                    if(label.contains(LHWMTypes::Fan, Qt::CaseInsensitive)) {
                        value = int(GPUSensor::FanSpeed);
                    }
                    break;
                }
                case ValueType::Wattage: {
                    value = int(GPUSensor::Wattage);
                    break;
                }
                case ValueType::Voltage:{
                    value = int(GPUSensor::Voltage);
                    break;
                }
                default:;
            }
            break;
        }
        case DeviceType::Storage:{
            break;
        }        
        case DeviceType::Memory:{
            break;
        }
        default:;
    }

    if(value > 10) {
        return AdjustedSensorID::adjusted(std::uint32_t(value));
    }

    return -1;
}

void HWSensorController::generateAvailableSensors(QJsonArray& available, QJsonObject settings, bool attach)
{
    auto hwMap{LHWM::GetHardwareSensorMap()};
    for (auto & [deviceName, deviceSensors] : hwMap)
    {
        QJsonObject chipObj;
        auto strDeviceName =  QString::fromLatin1(deviceName.data(), deviceName.size());
        chipObj.insert(SharedKeys::Name, strDeviceName);
        QJsonArray sensors;
        int dtype{-1};
        if(deviceSensors.size() == 0) {
            break;
        }
        for(auto const& sensorDesc : deviceSensors)
        {
            // name, type, path
            auto path = std::get<2>(sensorDesc);
            auto strPath = QString::fromStdString(path);
            auto strName = QString::fromStdString(std::get<0>(sensorDesc));
            if(strName.size() > 20) {
                auto words = strName.split(' ');
                switch(words.size()) {
                    case 1: {
                        strName = strName.left(20);
                        break;
                    }
                    case 2: {
                        strName = words.at(2).left(20);
                        break;
                    }
                    default: {
                        strName = words.at(1);
                        strName += " ";
                        strName += words.at(2);
                    }
                }
            }
            auto strType = QString::fromStdString(std::get<1>(sensorDesc));
            if(strType.compare(LHWMTypes::Control) == 0) {
                continue;
            }
            auto vtype = lhwmTypeToValueType(strType);
            QJsonValue sensorValue = QJsonValue::Null;
            QJsonValue deviceValue = QJsonValue::Null;
            auto adjustedSensor{-1};
            AdjustedSensorID id;
            if(dtype == -1) {
                dtype = guessDeviceType(strPath);
                chipObj.insert(SharedKeys::Type, dtype);
            }
            if(settings.size() <= 1) {
                // do the automap thing
                adjustedSensor = suggestSensorDefaultMap(strType, strName, strPath, vtype, dtype);
            }else if(settings.contains(strPath)){ // correct and valid mapping found
                adjustedSensor = settings.value(strPath).toInt(-1);
            }
            if(adjustedSensor != -1) {
                id.setAdjusted(adjustedSensor);
                auto sensor = watchingSensorID(adjustedSensor);
                if(attach) {
                    while(sensor)
                    {
                        ++id.device;
                        sensor = watchingSensorID(id.adjusted());
                    }
                    auto adj = id.adjusted();
                    watchSensor(adj, strPath);
                    sensor = watchingSensorID(adj);
                }
                if(sensor) {
                    id.setAdjusted(sensor->sensor);
                }
                deviceValue = int(id.device);
                sensorValue = int(id.sensor);
            }
            QJsonArray attributes{
                QJsonObject{
                    {SharedKeys::Name,strName},
                    {SharedKeys::Path, strPath},
                    {SharedKeys::Value, LHWM::GetSensorValue(path)},
                    {SharedKeys::Sensor, sensorValue},
                    {SharedKeys::Device, deviceValue}
                }
            };


            QJsonObject sensor{
                {SharedKeys::Name, strName},
                {SharedKeys::Path, strPath},
                {SharedKeys::Type, vtype},
                {SharedKeys::Attributes, attributes}
            };
            sensors.append(sensor);
        }
        chipObj.insert(SharedKeys::Sensors, sensors);
        available.append(chipObj);
    }

}

void HWSensorController::initialize(QJsonObject settings)
{
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
        if(pollTimer != -1) {
            mPollTimer->setInterval(pollTimer);
        } else {
            mPollTimer->setInterval(3000);
        }
        QJsonArray deviceDefinitions;
        generateAvailableSensors(deviceDefinitions, settings, true);
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
    Q_UNUSED(id)
    Q_UNUSED(value)
    // lhwm does not allow writes
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
        try{
            auto value = LHWM::GetSensorValue(buffer);
            sensor = new SensorUpdate{id, value, buffer};
            mUpdateList->append(sensor);
            if(!mPollTimer->isActive()) {
                mPollTimer->start();
            }
        } catch(...) {
            qDebug() << "Failed to fetch value for " << buffer;
            delete []buffer;
            buffer = nullptr;
        }
    }
}

