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

class QQuickItemGrabResult;

class QUsbDevice;
const int IMAGE_FRAME_SIZE = 409600;
struct  TempPoint{
    quint8  temp;
    quint8  point;
    TempPoint(quint8 temp, quint8 point) : temp(temp), point(point){}
    TempPoint(const TempPoint& rhs) : temp(rhs.temp), point(rhs.point){}
    TempPoint& operator=(const TempPoint&)= default;
};

class KrakenZDriver : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool  found READ found NOTIFY foundChanged MEMBER mFound)
    Q_PROPERTY( qreal liquidTemperature READ liquidTemperature NOTIFY liquidTemperatureChanged MEMBER mLiquidTemp)
    Q_PROPERTY( quint16 pumpSpeed READ pumpSpeed NOTIFY pumpSpeedChanged MEMBER mPumpSpeed)
    Q_PROPERTY( quint16 fanSpeed READ fanSpeed NOTIFY fanSpeedChanged MEMBER mFanSpeed)
    Q_PROPERTY( quint8 pumpDuty READ pumpDuty WRITE setPumpDuty NOTIFY pumpDutyChanged MEMBER mPumpDuty)
    Q_PROPERTY( quint8 fanDuty READ fanDuty WRITE setFanDuty NOTIFY fanDutyChanged MEMBER mFanDuty)
    Q_PROPERTY( QString version READ version NOTIFY versionChanged MEMBER mVersion)
    Q_PROPERTY( QString fwInfo READ fwInfo NOTIFY fwInfoChanged MEMBER mFwInfo)
    Q_PROPERTY( int rotationOffset READ rotationOffset WRITE setRotationOffset NOTIFY rotationOffsetChanged MEMBER mRotationOffset)
    Q_PROPERTY( qreal fps READ fps  NOTIFY fpsChanged MEMBER mFPS)
    Q_PROPERTY( quint8 brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged MEMBER mBrightness)
    Q_PROPERTY( short bucket READ bucket NOTIFY bucketChanged MEMBER mImageIndex)
    Q_PROPERTY( bool monitorFPS READ monitorFPS WRITE setMonitorFPS NOTIFY monitorFPSChanged)
public:
    explicit KrakenZDriver(QObject *parent = nullptr, quint16 VID = 0x1e71, quint16 PID = 0x3008);
    static quint16 calculateMemoryStart(quint8 index);
    qreal liquidTemperature() { return mLiquidTemp; }
    quint16 pumpSpeed() { return mPumpSpeed; }
    quint16 fanSpeed() { return mFanSpeed; }
    quint8 pumpDuty() { return mPumpDuty; }
    quint8 fanDuty() { return mFanDuty; }
    quint8 brightness() { return mBrightness; }
    QString version() { return mVersion; }
    QString fwInfo() { return mFwInfo; }
    short bucket() { return mImageIndex; }
    int     rotationOffset() { return mRotationOffset; }
    qreal fps() { return mFPS; }
    bool found() { return mFound; }
    bool monitorFPS() { return mMeasure.isActive(); }
    QJsonObject toJsonProfile();
    ~KrakenZDriver();
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

signals:
    void foundChanged(bool found);
    void fpsChanged(qreal fps);
    void liquidTemperatureChanged(qreal temperature);
    void pumpSpeedChanged(quint16 pump_rpm);
    void brightnessChanged(quint8 brightness);
    void fanSpeedChanged(quint16 fan_rpm);
    void pumpDutyChanged(quint8 duty_percent);
    void fanDutyChanged(quint8 duty_percent);
    void fwInfoChanged(QString fwInfo);
    void versionChanged(QString version);
    void deviceReady();
    void rotationOffsetChanged(int rotation);
    void error(QJsonObject response);
    void bucketChanged(quint8 bucket);
    void monitorFPSChanged(bool monitor);

public slots:
    void blankScreen();
    void initialize();
    void startMonitoringFramerate();
    void stopMonitoringFramerate();
    void setBrightness(quint8 brightness);
    void setFanDuty(quint8 duty);
    void setPumpDuty(quint8 duty); // flat
    void setImage(QImage image, quint8 index = 0, bool applyAfterSet = true);
    void setJsonProfile(QJsonObject profile);
    void setRotationOffset(int offset);
    void sendStatusRequest();
    void sendHex(QString hex_data, bool pad = true);
    void moveToBucket( int bucket = 0);
    void setNZXTMonitor();
    void setBuiltIn(quint8 index);
    void setScreenOrientation(Qt::ScreenOrientation orientation);
    void setMonitorFPS(bool monitor = true);
    void sendFWRequest();

protected slots:
    void receivedControlResponse();
    void updateFrameRate();

protected:
    void parseFWVersion(QByteArray& data);
    void parseStatus(QByteArray& data);
    void parseDeleteBucket(QByteArray& data);

    // Control messages
    void sendBrightness(quint8 brightness);
    void sendFanDuty(quint8 duty);
    void sendPumpDuty(quint8 duty);
    void sendQueryBucket(quint8 index, quint8 asset = 0);
    void sendDeleteBucket(quint8 index);
    void sendSwitchBucket(quint8 index, quint8 mode = 4);
    void sendSwitchLiquidTempMode();
    void sendSetupBucket(quint8 index, quint8 id, quint16 memory_slot, quint16 memory_slot_count);
    void sendWriteStartBucket(quint8 index);
    void sendWriteFinishBucket(quint8 index);
    void sendSetDutyProfile(quint8 channel, const QList<TempPoint>& profile);

    // Bulk Transfer CTRL Message
    void sendBulkDataInfo(quint8 mode = 2, quint32 size = 3276800);

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

#endif // KRAKENZDRIVER_H
