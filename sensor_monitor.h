#ifndef SENSORMONITOR_H
#define SENSORMONITOR_H

#include <QQuickItem>
class SystemMonitor;
// Sensor Monitor exposes a declarative interface to SystemMonitor.mappedSensors
// Sensors can be queried using mappedType property, and the deviceCount
// ie. type: CPU.packageTemp; device:1 - attaches to the first CPU.packageTemp
// other Qml items can then attach to the (initialized) property to determine when the sensor
// is ready and use the (value) property to read the sensors cached data
// other properties and functions are convenience
class SensorMonitor : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged MEMBER mValid)
    Q_PROPERTY(bool writable READ isWritable NOTIFY writableChanged MEMBER mWritable)
    Q_PROPERTY(int device READ device WRITE setDevice NOTIFY deviceChanged MEMBER mDevice)
    Q_PROPERTY(int sensor READ sensor WRITE setSensor NOTIFY sensorChanged MEMBER mSensor)
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged MEMBER mValue)

public:
    SensorMonitor(QQuickItem* parent = nullptr);

    // property reads
    bool valid() {return mValue;}
    bool isWritable() { return mWritable;}
    int device() {return mDevice;}
    int sensor() {return mSensor;}
    double value() {return mValue;}
    friend class SystemMonitor;
signals:
    void validChanged(bool valid);
    void writableChanged(bool writable);
    void deviceChanged(int device);
    void sensorChanged(int sensorID);
    void valueChanged(double value);

public slots:
    void setDevice(int device);
    void setSensor(int sensorID);
    void setValue(double value); // always available but might fail if enpoint is readonly

protected:
    bool    mValid;
    bool    mWritable;
    int     mSensor;
    int     mDevice;
    double  mValue;
    void createConnection();
    void releaseConnection();
    void setWritable(bool writable);

protected slots:
    void valueUpdated(double value);

};

#endif // SENSORMONITOR_H
