#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <QObject>
#include <QMap>
#include <QThread>
#include <QJsonArray>
#include <QJsonObject>
#include <string>
#include <QQuickItem>
#include <QMetaObject>
#include <QMetaEnum>
#include <QString>
#include "kzp_keys.h"
using namespace SharedKeys;

class QTimer;
class QQmlProperty;
class HWSensorController;
class SensorMonitor; // Qml interface

namespace KZP{
static constexpr char RELEASE[] = "releaseSensor";
static constexpr char WATCH[] = "watchSensor";
static constexpr char WRITE[] = "writeSensor";
static constexpr char INIT[] = "initialize";
static constexpr char REQUEST[] = "requestAvailableSensors";
static constexpr char UPDATE[] = "updateSensor";
};

typedef int AdjustedID;
// updates
// SensorUpdate becomes Struct;
struct SensorUpdate{
    ~SensorUpdate(){
        if(path) {
            delete [] path;
            path = nullptr;
        }
    }
    AdjustedID     sensor;
    double         value;
    char*          path = nullptr;
};

//typedef std::tuple<AdjustedID, double, QString>  SensorUpdate; // pair ID->Value
typedef QList<SensorUpdate*>  UpdateList;   // list of updates, stored in thread and reference passed to main thread

namespace KZPSensors{
    class DeviceTypes {
        Q_GADGET
        QML_ANONYMOUS
    public:
        enum class Device{
            CPU = 86,
            GPU = 102,
            Motherboard = 126,
            Storage = 134,
            Memory = 198,
            Network = 230,
            Unknown = -5
        };
        Q_ENUM(Device)
    private:
        explicit DeviceTypes(){}
    };

    class CPU : public QObject {
        Q_OBJECT
    public:
        enum class CPU_Sensor{
            DieTemp = 230,
            PackageTemp = 246,
            AverageTemp = 262,
            WorkLoad = 278,
            WorkLoadTotal = 279,
            Frequency = 294,
        };
        Q_ENUM(CPU_Sensor)
    private:
        explicit CPU(){}
    };
    class GPU : public QObject {
        Q_OBJECT
    public:
        enum class GPU_Sensor{
          CoreTemp = 310,
          JunctionTemp = 334,
          MemoryTemp = 358,
          MemoryUsed = 359,
          WorkLoad = 382 ,
          MemorySpeed = 383,
          FanSpeed = 384,
          ClockSpeed = 385,
          Frequency = 406,
          Voltage = 407,
          Wattage = 408
        };
        Q_ENUM(GPU_Sensor)
     private:
        explicit GPU(){}
    };
    class Storage : public QObject {
    public:
        enum class Storage_Info{
            Temperature = 504,
            Capacity = 510,
            UsedPercentage = 514,
            DataRate = 516,
            TransferRate = 518
        };
    };

    class Memory : public QObject {
    public:
        enum class Memory_Value{

        };
    };
    class ValueTypes: public QObject {
        Q_OBJECT
    public:
        enum class SuffixType {
            Celcius = 5337,
            Percentage,
            Value,
            RPM,
            YoctoVoltage,
            Megahertz,
            Megabytes,
            Gigahertz,
            Gigabytes
        };
        Q_ENUM(SuffixType)
        enum class ValueType {
            Temperature = 6337,
            Utilization,
            Speed,
            DataSize,
            Frequency,
            Voltage,
            Wattage,
            Current,
            Minimum,
            Maximum,
            Value,
            Highest,
            Lowest,
            TypeID,
            Data,
            DataRate
        };
        Q_ENUM(ValueType)
    private:
        explicit ValueTypes(){}
    };

    // Motherboard
    // Memory
    // Storage
} // End namespace KZPSensors


struct AdjustedSensorID{
    explicit AdjustedSensorID(int s = 0, int d = 0) : sensor(s), device(d)  {}
    std::uint32_t sensor:24;
    std::uint32_t device:8;
    int adjusted() const{
        return (sensor << 8) + device;
    }
    static int adjusted(uint32_t s, uint32_t d = 0){
        return (s << 8) + d;
    }
    void setAdjusted(int data)
    {
        sensor = (data >> 8);
        device = data & 0xff;
    }
};

inline bool operator==(const AdjustedSensorID& left, const AdjustedSensorID& right)
{
    return left.sensor == right.sensor && left.device == right.device;
}

class SystemMonitor;
class SensorDefinition : public QObject{
    Q_OBJECT
    friend class SystemMonitor;
    Q_PROPERTY(double value READ value NOTIFY valueChanged MEMBER mValue)
public:
    SensorDefinition(QObject* parent = nullptr) : QObject(parent), mValue(0.0), mWritable(false) {} // default empty
    SensorDefinition(QString name_in, int type_in, QString path_in, QString subfix_in = QString{}, QObject* parent = nullptr) :
        QObject(parent), mName(name_in), mType(type_in), mValue(0.0), mPath(path_in),  mSubfix(subfix_in), mWritable(false)
    {}
    QString name() { return mName; }
    int     type() { return mType; }
    double  value() { return mValue; }
    QString path() { return mPath; }
    QString subfix() { return mSubfix; }
    bool    canWrite() {return mWritable; }
public slots:

signals:
    void valueChanged(double value);

protected:
    void setWritable(bool canWrite) {
        mWritable = canWrite;
    }
    QString  mName;
    int      mType;
    double   mValue;
    QString  mPath;
    QString  mSubfix;
    bool     mWritable;

protected slots:
    void setValue(double v) {
        if(v != mValue) {
            mValue = v;
            emit valueChanged(mValue);
        }
    }
};

struct DeviceDefinition{
    explicit DeviceDefinition(KZPSensors::DeviceTypes::Device t = KZPSensors::DeviceTypes::Device::Unknown) : type(t){}
    QString                         name;
    KZPSensors::DeviceTypes::Device type;
    QList<SensorDefinition*>        sensors;
private: // do not permit copies
    DeviceDefinition(const DeviceDefinition& rvalue) { Q_UNUSED(rvalue)}
    DeviceDefinition& operator=(const DeviceDefinition& rvalue) { Q_UNUSED(rvalue); return *this;}
    // allow moves
    DeviceDefinition& operator=(DeviceDefinition&& rvalue)
    {
        name = rvalue.name; type = rvalue.type; sensors = rvalue.sensors; // implicit copy
        rvalue.name = QLatin1String{};
        rvalue.type = KZPSensors::DeviceTypes::Device::Unknown;
        rvalue.sensors = QList<SensorDefinition*>{};
        return *this;
    }
};

typedef std::pair<QObject*,QQmlProperty*> MappedProperty;
typedef std::pair<double, QList<MappedProperty>* > MappedSensor;
typedef QList<DeviceDefinition*>* DeviceList;
typedef QMap<int, SensorDefinition* > SensorMap;
typedef SensorMap* MappingList;


// Singleton Ux interface object
class SystemMonitor : public QObject
{
    Q_OBJECT
   // Q_PROPERTY(quint32 pollDelay READ pollDelay WRITE setPollDelay NOTIFY pollDelayChanged MEMBER mPollDelay)
public:
    explicit SystemMonitor(QObject *parent = nullptr);
    ~SystemMonitor();


    // Loading from settings json object
    Q_INVOKABLE QJsonObject jsonSettings();


    // NEW Interface
    // App interface
    Q_INVOKABLE bool isReady() { return (mDeviceList && mDeviceList->size()>0); }
    Q_INVOKABLE bool isAttached(int sensorID, int device = 0);
    Q_INVOKABLE bool connectOn(int sensorID, int device, QObject& item);
    Q_INVOKABLE bool releaseConnection(int sensorID, int device, QObject& item);
    Q_INVOKABLE double readSensor(int sensorID, int device = 0, bool * valid = nullptr);
    Q_INVOKABLE void   writeToSensor(int sensorID, int device = 0, double value = 0.0, bool* written = nullptr);
    Q_INVOKABLE bool   sensorExists(int sensor, int device = 0);
    Q_INVOKABLE void   queryDevicesAvailable();
    // Sensor Convenienve Methods
    using CPU_Sensor = KZPSensors::CPU::CPU_Sensor;
    using GPU_Sensor = KZPSensors::GPU::GPU_Sensor;
    using ValueType = KZPSensors::ValueTypes::ValueType;
    Q_INVOKABLE static QString sensorIconSource(int type);
    Q_INVOKABLE static bool isValidSensor(int sensorID);
    Q_INVOKABLE static QJsonArray sensorsAvailable();
    Q_INVOKABLE static QJsonArray sensorValueTypes();
    Q_INVOKABLE static QString sensorIDToString(int type);
    Q_INVOKABLE static QString sensorTypeToString(int type);

    void initializeSensors(QJsonObject sensorMap);

    void aboutToExit();
signals:
    void sensorsReady();
    void availableDevices(QJsonArray devices);

protected:
    // improve API and add declarative class

    HWSensorController*          mSensorController;
    QThread                      mSensorThread;
    // mAttached sensors utilize the SensorDefinitions inside the Device Definitions
    SensorMap*                   mAttachedSensors; // Map for updates
    // All Device Definitions will exist within the mDeviceList
    QList<DeviceDefinition*>*    mDeviceList; // Lookup list
    quint32                      mPollDelay;
    QJsonObject                  mSettings;

//    QStringList                  mDevices;
//    QStringList                  mTypes;
//    QMap<QString, MappedSensor>  mMap;

    void clearCache();
    void initializeHwSensorController();
    void releseHWSensorController();

protected slots:
    void clearAttachedMap();
    void clearDeviceList();
    void releaseAll();
    void receivedDevices(QJsonArray deviceArray);
    void onSensorMapUpdate(QJsonArray deviceArray);
    void onSensorUpdate(void* updates);
    void onUpdate(int sensorID, double value);
};


#endif // SYSTEM_MONITOR_H
