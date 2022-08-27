#include "sensor_monitor.h"
#include "kzp_controller.h"
#include <QDebug>
SensorMonitor::SensorMonitor(QQuickItem* parent)
    : QQuickItem(parent), mValid(false), mWritable(false), mSensor(0), mDevice(0), mValue(0)
{

    auto systemMonitor{ KZPController::getSystemMonitor()}; // global instance
    if(systemMonitor) {
        connect(systemMonitor, &SystemMonitor::sensorsReady, this, &SensorMonitor::createConnection);
    }
}



void SensorMonitor::createConnection()
{
    auto systemMonitor{ KZPController::getSystemMonitor()}; // global instance
    if(!systemMonitor || mSensor == 0) // not configured
        return;

    if(systemMonitor->isReady() && systemMonitor->connectOn(mSensor, mDevice, *this)){
        mValid = true;
        emit validChanged(true);
        bool validRead{false};
        auto v{systemMonitor->readSensor(mSensor, mDevice, &validRead)};
        if(validRead) {
            mValue = v;
            emit valueChanged(mValue);
        }
    } else {
        if(mValid) {
            mValid = false;
            emit validChanged(mValid);
        }
    }


}

void SensorMonitor::setDevice(int device)
{
    if(device >= 0 && device != mDevice) {
        mDevice = device;
        emit deviceChanged(device);
        releaseConnection();
        createConnection();
        return;
    }
    mValid = false;
}

void SensorMonitor::releaseConnection()
{
    if(mValid) {
        auto systemMonitor{ KZPController::getSystemMonitor()};
        if(systemMonitor) {
            systemMonitor->releaseConnection(mSensor, mDevice, *this);
        }
    }
}

void SensorMonitor::setValue(double value)
{
    if(mWritable) {
         auto systemMonitor{ KZPController::getSystemMonitor()};
         if(systemMonitor) {
             systemMonitor->writeToSensor(mSensor, mDevice, value);
         }
    }
}


void SensorMonitor::setSensor(int sensorID)
{
    if(sensorID >= 0 && sensorID != mSensor) {
        mSensor = sensorID;
        emit deviceChanged(mSensor);
        releaseConnection();
        createConnection();
        return;
    }
    mValid = false;
}

void SensorMonitor::setWritable(bool writable)
{
    if(mWritable != writable) {
        mWritable = writable;
        emit writableChanged(mWritable);
    }
}

void SensorMonitor::valueUpdated(double value)
{
    if(mValue != value) {
        mValue = value;
        emit valueChanged(value);
    }
}
