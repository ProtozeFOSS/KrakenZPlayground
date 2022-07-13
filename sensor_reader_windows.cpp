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

#include <map>
using std::map;

#include <lhwm-cpp-wrapper.h>


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

void HWSensorReader::cleanUp()
{

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




