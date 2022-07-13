#include "system_monitor.h"
#include <string_view>
#include <iostream>
#include "sensors-c++/sensors.h"
#include <QDebug>
#include <QTimer>


void HWSensorReader::checkSensors()
{
    if(mUpdateList && mUpdateList->size() > 0) {
        for(int i{0}; i < mUpdateList->size(); ++i){
            auto sensor{mUpdateList->at(i)};
            sensors::subfeature f(std::string_view(sensor->first));
            sensor->second = f.read();
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

QString featureTypeToQString(sensors::feature_type t)
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

void HWSensorReader::generateSensorMap()
{
    // get list of available sensors
    auto chips = sensors::get_detected_chips();
    auto devices{new QList<DeviceDefinition*>};
    // for chips
    for(const auto& chip: chips) {
        qDebug() << "Found Chip: " << chip.name().data();
        qDebug() << "Path: " << chip.path().data();
        auto features = chip.features();
        if(features.size() > 0) {
            qDebug() << features.size() << " Features Available";
            auto device{new DeviceDefinition};
            auto size {chip.prefix().size()};
            auto buffer = new char[size];
            buffer[size-1] = '\0';
            memcpy(buffer,chip.prefix().data(),size);
            device->name = QString::fromLocal8Bit(buffer,size);
            for(const auto& feature : features) {
                qDebug() << "Found Feature: " << feature.name().data();
                auto subfeatures = feature.subfeatures();
                for(const auto& sub : subfeatures) {
                    if((sub.type() != sensors::subfeature_type::input || sub.type() != sensors::subfeature_type::input_highest || sub.type() != sensors::subfeature_type::input_lowest) && sub.readable()) {
                        qDebug() << "Found SubFeature: " << sub.name().data();
                        auto sdef = new SensorDefinition(QString::fromLocal8Bit(sub.name().data(), sub.name().size()),
                                                         featureTypeToQString(feature.type()),
                                                         QString::fromLocal8Bit(chip.path().data(), chip.path().size()) + "/" + QString::fromLocal8Bit(sub.name().data(), sub.name().size()));
                        qDebug() << "Reading: " << sdef->path;
                        try{
                            sensors::subfeature f(std::string_view(sdef->path.toStdString()));
                            qDebug() << "Value: " <<  f.read();
                            qDebug() << "Adding: " << sub.name().data() ;
                            device->sensors.push_back(sdef);
                        }catch(...) {
                            qDebug() << "Failed to read " << sdef->path;
                        }
                    }
                }
            }
            devices->append(device);
        }
        qDebug() << "\n";
    }
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
//    QTimer::singleShot(10, mPollTimer, [this](){
//        auto aed{QAbstractEventDispatcher::instance()};
//        aed->processEvents(QEventLoop::AllEvents);
//        mPollTimer->start();
//    });
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
            sensors::subfeature sub(buffer);
            mUpdateList->append(new SensorUpdate(buffer, sub.read()));
            mPollTimer->start();
        });
    }
}
