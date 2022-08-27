#ifndef KRAKENZ_INTERFACE_H
#define KRAKENZ_INTERFACE_H

#include <QObject>
#include <QJsonObject>
#include <QImage>


struct  TempPoint{
    quint8  temp;
    quint8  point;
    TempPoint(quint8 temp, quint8 point) : temp(temp), point(point){}
    TempPoint(const TempPoint& rhs) : temp(rhs.temp), point(rhs.point){}
    TempPoint& operator=(const TempPoint& rhs)
    {
        temp = rhs.temp;
        point = rhs.point;
        return *this;
    };
};


class KrakenZInterface : public QObject{
    Q_OBJECT
    Q_PROPERTY( bool  found READ found NOTIFY foundChanged)
    Q_PROPERTY( qreal liquidTemperature READ liquidTemperature NOTIFY liquidTemperatureChanged)
    Q_PROPERTY( quint16 pumpSpeed READ pumpSpeed NOTIFY pumpSpeedChanged)
    Q_PROPERTY( quint16 fanSpeed READ fanSpeed NOTIFY fanSpeedChanged)
    Q_PROPERTY( quint8 pumpDuty READ pumpDuty WRITE setPumpDuty NOTIFY pumpDutyChanged)
    Q_PROPERTY( quint8 fanDuty READ fanDuty WRITE setFanDuty NOTIFY fanDutyChanged)
    Q_PROPERTY( QString version READ version NOTIFY versionChanged)
    Q_PROPERTY( int rotationOffset READ rotationOffset WRITE setRotationOffset NOTIFY rotationOffsetChanged)
    Q_PROPERTY( qreal fps READ fps  NOTIFY fpsChanged)
    Q_PROPERTY( quint8 brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY( short bucket READ bucket NOTIFY bucketChanged)
    Q_PROPERTY( bool isSoftware READ isSoftware CONSTANT)

public:
    KrakenZInterface(QObject* parent = nullptr) : QObject(parent){}
    enum WriteTarget{
        NO_TARGET = 0x00,
        FW_INFO = 0x10,
        FW_REQUEST = 0x1,
        RESPONSE_VERSION = 0x11,
        QUERY_BUCKET = 0x30,
        UNKNOWN_SUB = 0x04, // mode?
        QUERY_RESPONSE = 0x31,
        SETUP_BUCKET = 0x32,
        SET_BUCKET = 0x1,
        DELETE_BUCKET = 0x2,
        SETUP_RESPONSE = 0x33,
        WRITE_SETUP = 0x36,
        WRITE_START = 0x1,
        WRITE_FINISH = 0x2,
        BRIGHTNESS = 0x2,
        UNKNOWN_MSG_1 = 0x20,
        UNKNOWN_MSG_1SUB1 = 0x03,
        UNKNOWN_RSP_1 = 0x21,
        UNKNOWN_RSP_1SUB1 = 0x03,
        UNKNOWN_MSG_2 = 0x70,
        UNKNOWN_MSG_2SUB1 = 0x01,
        UNKNOWN_RSP_2 = 0x71,
        UNKNOWN_RSP_2SUB1 = 0x01,
        UNKNOWN_MSG_3 = 0x2a,
        UNKNOWN_MSG_3SUB1 = 0x04,
        CONFIRM_RESPONSE = 0xff,
        CONFIRM_SUCCESS = 0x01,
        CONFIRM_FAIL = 0x02,
        RESPONSE_WRITE = 0x37,
        RESPONSE_WRITE_START = 0x1,
        RESPONSE_WRITE_FINISH = 0x2,
        SWITCH_BUCKET = 0x38,
        RESPONSE_SWITCH_BUCKET = 0x39,
        SET_DUTY = 0x72,
        AIO_STATUS = 0x74,
        RESPONSE_STATUS = 0x75,
    };
    enum ImageTransferState{
        SELECT_TARGET = 0x00,
        SEND_QUERY_BUCKET,
        SEND_WRITE_SETUP,
        SEND_WRITE_START,
        SEND_BULK_DATA,
        SEND_WRITE_FINISH,
        SEND_SWITCH
    };
    enum Channel {
        PUMP_CHANNEL = 0x1,
        FAN_CHANNEL = 0x2
    };
    enum OperationMode {
        DISPLAY_BLACK = 0x00,
        BUILT_IN = 0x01,
        LIQUID_MONITOR = 0x02,
        ASSET_MODE = 0x03,
        BUCKET_MODE = 0x04
    };

    Q_ENUM(WriteTarget)
    virtual qreal liquidTemperature() { return 30; }
    virtual quint16 pumpSpeed() { return 1000; }
    virtual quint16 fanSpeed() { return 1000; }
    virtual quint8 pumpDuty() { return 50; }
    virtual quint8 fanDuty() { return 50; }
    virtual quint8 brightness() { return 50; }
    virtual QString version() { return QStringLiteral("v.interface.class"); }
    virtual short bucket() { return 0; }
    virtual int     rotationOffset() { return 0; }
    virtual qreal fps() { return 0; }
    virtual bool found() { return false; }
    virtual bool initialize(bool&){ return false;}
    virtual bool initialized() { return false; }
    virtual bool isSoftware() { return true; }
    virtual QJsonObject toJsonProfile();

signals:
    void foundChanged(bool);
    void fpsChanged(qreal);
    void liquidTemperatureChanged(qreal);
    void pumpSpeedChanged(quint16);
    void brightnessChanged(quint8);
    void fanSpeedChanged(quint16);
    void pumpDutyChanged(quint8);
    void fanDutyChanged(quint8);
    void versionChanged(QString);
    void deviceReady();
    void rotationOffsetChanged(int);
    void bucketChanged(quint8);

public slots:
    virtual void blankScreen(){ setBrightness(0); }
    virtual void moveToBucket( int){}
    virtual void startMonitoringFramerate(){}
    virtual void stopMonitoringFramerate(){}
    virtual void setBrightness(quint8){}
    virtual void setFanDuty(quint8){}
    virtual void setPumpDuty(quint8){} // flat
    virtual void setImage(QImage, quint8 = 0, bool = true){}
    virtual void setJsonProfile(QJsonObject);
    virtual void setRotationOffset(int){}
    virtual void setNZXTMonitor(){}
    virtual void setBuiltIn(quint8){}
    virtual void setScreenOrientation(Qt::ScreenOrientation){}
};

Q_DECLARE_INTERFACE(KrakenZInterface, "com.application.kzp")



class KrakenZDriver;
class KrakenZSoftware;
class KrakenZDriverSelect;

extern KrakenZDriverSelect* KRAKEN_DRIVER_SELECT;

class KrakenZDriverSelect: public QObject{

    Q_OBJECT
public:
    enum DriverType{
        SOFTWARE=0,
        HARDWARE=1
    };
    Q_ENUM(DriverType)
    KrakenZInterface* currentDriver();
    DriverType driverType() { return mDriverType; }
    static void initializeDriverSelect(QApplication* app, KrakenZDriverSelect::DriverType type);
    static KrakenZDriverSelect* GetInstance();

protected:    
    KrakenZDriverSelect(QApplication* app, KrakenZDriverSelect::DriverType type = DriverType::HARDWARE);
    ~KrakenZDriverSelect();
    void releaseSoftwareDriver();
    void releaseHardwareDriver();

public:
    void releaseDrivers() // can release all resources before destructor
    {
        releaseSoftwareDriver();
        releaseHardwareDriver();
    }
    Q_INVOKABLE void selectDriver(KrakenZDriverSelect::DriverType type);

signals:
    void driverChanged(QObject* driver);

protected:
    DriverType       mDriverType;
};

#endif // KRAKENZ_INTERFACE_H
