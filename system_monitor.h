#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <QObject>
#include <QMap>
#include <QThread>
#include <QJsonArray>
#include <QJsonObject>
#include <string>
class QTimer;
class QQmlProperty;
class QQuickItem;

static constexpr char RELEASE[] = "releaseSensor";
static constexpr char MONITOR[] = "startMonitoring";
static constexpr char WATCH[] = "watchSensor";

typedef std::pair<const char*, double>  SensorUpdate;
typedef QList<SensorUpdate*> UpdateList;


struct SensorDefinition{
    SensorDefinition(QString name_in, QString type_in, QString path_in)
    {
        name = name_in;
        type = type_in;
        path = path_in;
    }
    ~SensorDefinition(){}
    QString  name;
    QString  type;
    QString  path;
};

struct DeviceDefinition{
    ~DeviceDefinition(){name.clear(); sensors.clear();}
    QString                         name;
    std::vector<SensorDefinition*>  sensors;
};


class HWSensorReader: public QObject
{
    Q_OBJECT

public:
    HWSensorReader(QObject* parent = nullptr)
        : QObject{parent}, mUpdateList{nullptr}, mPollTimer{nullptr}
    {}
    ~HWSensorReader()
    {
        cleanUp();
        mPollTimer = nullptr; // was deleted from thread exit
    }
signals:
    void sensorUpdate(void* updates);
    void sensorMapUpdate(void* devices);

public slots:
    void updateReceived();
    void startMonitoring(int pollDelay);
    void stopMonitoring();
    void watchSensor(const QString sensorPath);
    void releaseSensor(const QString sensorPath);
    void requestSensorMap();

protected slots:
    void checkSensors();
    void generateSensorMap();

protected:
    void cleanUp();
    void clearWatching();
    UpdateList*          mUpdateList;
    QTimer*              mPollTimer;
};

typedef std::tuple<QString, quint32, quint32> SensorData;
typedef std::pair<QObject*,QQmlProperty*> MappedProperty;
typedef std::pair<double, QList<MappedProperty>* > MappedSensor;

// Ux interface object
class SystemMonitor : public QObject
{
    Q_OBJECT
   // Q_PROPERTY(quint32 pollDelay READ pollDelay WRITE setPollDelay NOTIFY pollDelayChanged MEMBER mPollDelay)
public:
    explicit SystemMonitor(QObject *parent = nullptr);
    ~SystemMonitor();

    // 4 core types - CPU, GPU, STORAGE, MEMORY
//    Q_INVOKABLE quint8      cpuCount();
//    Q_INVOKABLE QJsonObject getCPU(quint8 cpuIndex); // core count, load[cores+1], temp[cores+1]

//    Q_INVOKABLE quint8      gpuCount();
//    Q_INVOKABLE QJsonObject getGPU(quint8 gpuIndex); // gpu info

//    Q_INVOKABLE quint8      storageCount();
//    Q_INVOKABLE QJsonObject getStorage(quint8 index); // storage info

//    Q_INVOKABLE QJsonObject memoryInfo(); // Physical MEM, Reserved

    Q_INVOKABLE bool connectOn(const QString& sensorPath, QObject *item, const QString& propertyName);
    Q_INVOKABLE bool connectOn(const QJsonObject& sensor, QObject *item, const QString& propertyName);
    Q_INVOKABLE QStringList sensorTypes() { return mTypes; }
    Q_INVOKABLE QJsonArray  sensorsByType(const QString& type, const QString &deviceFilter = QString());
    Q_INVOKABLE bool        releaseAll(QObject* item);
    //Q_INVOKABLE QString     suffixByType(QString type);
    Q_INVOKABLE bool        sensorExists(const QString& sensorPath);
    Q_INVOKABLE double      currentValue(const QString& sensorPath);
    Q_INVOKABLE bool        isReady() { return mDevices.size() > 0; }
    Q_INVOKABLE QStringList devices() { return mDevices; }

    void  aboutToExit();
signals:
    void sensorsReady();

protected:
    // improve API and add declarative class
    QStringList                  mDevices;
    QStringList                  mTypes;
    QMap<QString, SensorData>    mSensorMap;

    QMap<QString, MappedSensor>  mMap;
    HWSensorReader*              mReader;
    QThread                      mReaderThread;
    quint32                      mPollDelay;

    void clearCache();

protected slots:
    void onSensorMapUpdate(void* devices);
    void onSensorUpdate(void* updates);
    void onItemDestroyed();
};

#endif // SYSTEM_MONITOR_H
