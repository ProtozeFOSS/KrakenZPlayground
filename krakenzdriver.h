#ifndef KRAKENZDRIVER_H
#define KRAKENZDRIVER_H

#include <QObject>
#include <QJsonObject>
#include <QImage>
#include <QQuickItem>
#include <QSharedPointer>
#include <QTimer>
#include "qusbendpoint.h"
#include <QJsonObject>
#include <QApplication>


struct  TempPoint{
    quint8  temp;
    quint8  point;
    TempPoint(quint8 temp, quint8 point) : temp(temp), point(point){}
    TempPoint(const TempPoint& rhs) : temp(rhs.temp), point(rhs.point){}
    TempPoint& operator=(const TempPoint&)= default;
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
    Q_PROPERTY( QString fwInfo READ fwInfo NOTIFY fwInfoChanged)
    Q_PROPERTY( int rotationOffset READ rotationOffset WRITE setRotationOffset NOTIFY rotationOffsetChanged)
    Q_PROPERTY( qreal fps READ fps  NOTIFY fpsChanged)
    Q_PROPERTY( quint8 brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY( short bucket READ bucket NOTIFY bucketChanged)
    Q_PROPERTY( bool monitorFPS READ monitorFPS WRITE setMonitorFPS NOTIFY monitorFPSChanged)

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
    virtual QString fwInfo() { return QStringLiteral("v.fw.info"); }
    virtual short bucket() { return 0; }
    virtual int     rotationOffset() { return 0; }
    virtual qreal fps() { return 0; }
    virtual bool found() { return false; }
    virtual bool initialized() { return false; }
    virtual bool monitorFPS() { return false; }
    virtual QJsonObject toJsonProfile(){ return QJsonObject(); }

signals:
    void foundChanged(bool);
    void fpsChanged(qreal);
    void liquidTemperatureChanged(qreal);
    void pumpSpeedChanged(quint16);
    void brightnessChanged(quint8);
    void fanSpeedChanged(quint16);
    void pumpDutyChanged(quint8);
    void fanDutyChanged(quint8);
    void fwInfoChanged(QString);
    void versionChanged(QString);
    void deviceReady();
    void rotationOffsetChanged(int);
    void error(QJsonObject);
    void bucketChanged(quint8);
    void monitorFPSChanged(bool);

public slots:
    virtual void blankScreen(){}
    virtual void initialize(){}
    virtual void startMonitoringFramerate(){}
    virtual void stopMonitoringFramerate(){}
    virtual void setBrightness(quint8){}
    virtual void setFanDuty(quint8){}
    virtual void setPumpDuty(quint8){} // flat
    virtual void setImage(QImage, quint8 = 0, bool = true){}
    virtual void setJsonProfile(QJsonObject){}
    virtual void setRotationOffset(int){}
    virtual void sendStatusRequest(){}
    virtual void sendHex(QString, bool){}
    virtual void moveToBucket( int){}
    virtual void setNZXTMonitor(){}
    virtual void setBuiltIn(quint8){}
    virtual void setScreenOrientation(Qt::ScreenOrientation){}
    virtual void setMonitorFPS(bool = true){}
    virtual void sendFWRequest(){}

protected:
    virtual void parseFWVersion(QByteArray&){}
    virtual void parseStatus(QByteArray&){}
    virtual void parseDeleteBucket(QByteArray&){}

    // Control messages
    virtual void sendBrightness(quint8){}
    virtual void sendFanDuty(quint8){}
    virtual void sendPumpDuty(quint8){}
    virtual void sendQueryBucket(quint8, quint8){}
    virtual void sendDeleteBucket(quint8){}
    virtual void sendSwitchBucket(quint8, quint8){}
    virtual void sendSwitchLiquidTempMode(){}
    virtual void sendSetupBucket(quint8, quint8, quint16, quint16){}
    virtual void sendWriteStartBucket(quint8){}
    virtual void sendWriteFinishBucket(quint8){}
    virtual void sendSetDutyProfile(quint8, const QList<TempPoint>&){}
    // Bulk Transfer CTRL Message
    virtual void sendBulkDataInfo(quint8, quint32){}
};

class QQuickItemGrabResult;

class QUsbDevice;
const int IMAGE_FRAME_SIZE = 409600;

class KrakenZDriver : public KrakenZInterface
{
    Q_OBJECT
public:
    explicit KrakenZDriver(QObject *parent = nullptr, quint16 VID = 0x1e71, quint16 PID = 0x3008);
    static quint16 calculateMemoryStart(quint8 index);
    qreal liquidTemperature() override { return mLiquidTemp; }
    quint16 pumpSpeed() override { return mPumpSpeed; }
    quint16 fanSpeed() override { return mFanSpeed; }
    quint8 pumpDuty() override { return mPumpDuty; }
    quint8 fanDuty() override { return mFanDuty; }
    quint8 brightness() override { return mBrightness; }
    QString version() override { return mVersion; }
    QString fwInfo() override { return mFwInfo; }
    short bucket() override { return mImageIndex; }
    int     rotationOffset() override { return mRotationOffset; }
    qreal fps() override { return mFPS; }
    bool found() override { return mFound; }
    bool initialized() override { return mInitialized; }
    bool monitorFPS() override { return mMeasure.isActive(); }
    Q_INVOKABLE void closeConnections();
    QJsonObject toJsonProfile() override;
    ~KrakenZDriver();


public slots:
    void blankScreen() override;
    void initialize() override;
    void startMonitoringFramerate() override;
    void stopMonitoringFramerate() override;
    void setBrightness(quint8 brightness) override;
    void setFanDuty(quint8 duty) override;
    void setPumpDuty(quint8 duty) override; // flat
    void setImage(QImage image, quint8 index = 0, bool applyAfterSet = true) override;
    void setJsonProfile(QJsonObject profile) override;
    void setRotationOffset(int offset) override;
    void sendStatusRequest() override;
    void sendHex(QString hex_data, bool pad = true) override;
    void moveToBucket( int bucket = 0) override;
    void setNZXTMonitor() override;
    void setBuiltIn(quint8 index) override;
    void setScreenOrientation(Qt::ScreenOrientation orientation) override;
    void setMonitorFPS(bool monitor = true) override;
    void sendFWRequest() override;

protected slots:
    void receivedControlResponse();
    void updateFrameRate();

protected:
    void parseFWVersion(QByteArray& data) override;
    void parseStatus(QByteArray& data) override;
    void parseDeleteBucket(QByteArray& data) override;

    // Control messages
    void sendBrightness(quint8 brightness) override;
    void sendQueryBucket(quint8 index, quint8 asset = 0) override;
    void sendDeleteBucket(quint8 index) override;
    void sendSwitchBucket(quint8 index, quint8 mode = 4) override;
    void sendSwitchLiquidTempMode() override;
    void sendSetupBucket(quint8 index, quint8 id, quint16 memory_slot, quint16 memory_slot_count) override;
    void sendWriteStartBucket(quint8 index) override;
    void sendWriteFinishBucket(quint8 index) override;

    // Bulk Transfer CTRL Message
    void sendBulkDataInfo(quint8 mode = 2, quint32 size = 3276800) override;

    bool           mFound;
    bool           mInitialized;
    QUsbDevice*    mKrakenDevice; // Single composite usb device handle
    QUsbEndpoint*  mLCDDATA;
    QUsbEndpoint*  mLCDCTL;
    QUsbEndpoint*  mLCDIN;

    // Pump/Fan Qml Properties
    qreal   mLiquidTemp;
    quint16 mFanSpeed;
    quint16 mPumpSpeed;
    quint8  mFanDuty;
    quint8  mPumpDuty;
    QString mVersion;
    QString mFwInfo;
    quint8  mBrightness;
    int     mRotationOffset; // lcd rotation offset

    // Write Buffer
    QString             mFilePath;
    QSharedPointer<QQuickItemGrabResult> mResult;
    short               mBufferIndex; // buffer index
    short               mImageIndex; // bucket id
    QTimer              mMeasure;
    qint64              mBytesLeft;
    quint64             mBytesSent;
    char                mFrameOut[IMAGE_FRAME_SIZE];
    QImage              mImageOut;
    bool                mApplyAfterSet;
    bool                mWritingImage;
    short               mFrames;
    quint32             mFrameDelay;
    qreal               mFPS;

};

static KrakenZInterface*  CURRENT_KRAKENZ_DRIVER = nullptr;
static KrakenZInterface*  HARDWARE_DRIVER = nullptr;
static KrakenZInterface*  SOFTWARE_DRIVER = nullptr;

class KrakenZSoftware: public KrakenZInterface {
    Q_OBJECT
public:
    KrakenZSoftware(QObject* parent = nullptr) : KrakenZInterface(parent){};
};

class KrakenZDriverSelect: public QObject{

    Q_OBJECT
public:
    enum DriverType{
        SOFTWARE=0,
        HARDWARE=1
    };
    Q_ENUM(DriverType)
    KrakenZDriverSelect(QApplication* app, KrakenZDriverSelect::DriverType type = DriverType::HARDWARE) : QObject{app}, mApp{app}
    {
        selectDriver(type);
    }
    KrakenZInterface* currentDriver() { return CURRENT_KRAKENZ_DRIVER; }
    void setCurrentDriver(KrakenZInterface* driver) // C++ only
    {
        if(driver != CURRENT_KRAKENZ_DRIVER){
            CURRENT_KRAKENZ_DRIVER = driver;
            emit driverChanged(driver);
        }
    }
protected:

    void releaseSoftwareDriver()
    {
        delete SOFTWARE_DRIVER;
        SOFTWARE_DRIVER = nullptr;
    }
    void releaseHardwareDriver()
    {
        delete HARDWARE_DRIVER;
        HARDWARE_DRIVER = nullptr;
    }
public:
    void releaseDrivers() // can release all resources before destructor
    {
        releaseSoftwareDriver();
        releaseHardwareDriver();
    }
    Q_INVOKABLE void selectDriver(KrakenZDriverSelect::DriverType type)
    {
        if(type == DriverType::SOFTWARE){
            if(!SOFTWARE_DRIVER){
                SOFTWARE_DRIVER = new KrakenZSoftware{mApp};
            }
            releaseHardwareDriver();
            setCurrentDriver(SOFTWARE_DRIVER);
        } else {
            if(!HARDWARE_DRIVER){
                HARDWARE_DRIVER = new KrakenZDriver{mApp};
            }
            releaseSoftwareDriver();
            setCurrentDriver(HARDWARE_DRIVER);
        }
    }

signals:
    void driverChanged(QObject* driver);

protected:
    QApplication*    mApp;
};

#endif // KRAKENZDRIVER_H
