#ifndef SENSOR_CONTROLLER_H
#define SENSOR_CONTROLLER_H
#include <QObject>
#include "system_monitor.h"

class HWSensorController: public QObject
{
    Q_OBJECT

public:
    HWSensorController(QObject* parent = nullptr)
        : QObject{parent}, mUpdateList{nullptr}, mPollTimer{nullptr}
    {}
    friend class SystemMonitor;
signals:
    // See AdjustedSensorID struct in SystemMonitor
    // updates -> QMap<adjustedSensorID, double>; // send sensor read values
    void sensorUpdate(void* updates); // pass mUpdateList
    // send details of chip and underlying features
    // QJsonArray deviceList
    void sensorsAvailable(QJsonArray devices);
    // Parameter 1 -> QList<DeviceDefinitions*>*
    // Paremeter 2 QMap<adjustedSensorID, SensorDefinition*>*
    void sensorMapGenerated(QJsonArray devices);

public slots:
    void stopMonitoring();
    void watchSensor(AdjustedID sensor, const QString sensorPath);
    void releaseSensor(AdjustedID id);
    void requestAvailableSensors(QJsonObject settings);

protected slots:
    void initialize(QJsonObject settings);
    void writeSensor(AdjustedID sensor, double value);
    void checkSensors();
    SensorUpdate* watchingSensor(const QString& path) // find sensor if being monitored, or return null
    {
        SensorUpdate* ret{nullptr};
        if(path.size() > 0) {
            for(auto iter{mUpdateList->begin()}; iter != mUpdateList->end(); iter++) {
                if(path.compare((*iter)->path) == 0) {
                    ret = (*iter);
                    break;
                }
            }
        }
        return ret;
    }
    SensorUpdate* watchingSensorID(AdjustedID id)
    {
        SensorUpdate* ret{nullptr};
        if(id > 0) {
            for(auto iter{mUpdateList->begin()}; iter != mUpdateList->end(); iter++) {
                if((*iter)->sensor == id) {
                    ret = (*iter);
                    break;
                }
            }
        }
        return ret;
    }

protected:
    void  generateAvailableSensors(QJsonArray& available, QJsonObject settings = QJsonObject{}, bool attach = false);
    UpdateList*    mUpdateList;
    QTimer*        mPollTimer;
};

#endif // SENSOR_CONTROLLER_H
