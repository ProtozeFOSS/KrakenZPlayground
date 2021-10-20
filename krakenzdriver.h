#ifndef KRAKENZDRIVER_H
#define KRAKENZDRIVER_H

#include <QObject>
#include <QJsonObject>
#include <QImage>
#include <QQuickItem>
#include <QSharedPointer>
#include <QTimer>
#include "qusbendpoint.h"
class QQuickItemGrabResult;

class QUsbDevice;
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
    Q_PROPERTY( QQuickItem* content READ content WRITE setContent NOTIFY contentChanged MEMBER mContent)
    Q_PROPERTY( quint8 brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged MEMBER mBrightness)
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
    int     rotationOffset() { return mRotationOffset; }
    QQuickItem* content() { return mContent; }
    qreal fps() { return mFPS; }
    bool found() { return mFound; }
    ~KrakenZDriver();
    enum WriteTarget{
        NO_TARGET = 0x00,
        FW_VERSION = 0x10,
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
    enum CHANNELS{
        PUMP_CHANNEL = 0x1,
        FAN_CHANNEL = 0x2
    };

    Q_ENUM(WriteTarget)

signals:
    void foundChanged(bool found);
    void fpsChanged(qreal fps);
    void contentChanged(QQuickItem* content);
    void liquidTemperatureChanged(qreal temperature);
    void pumpSpeedChanged(quint16 pump_rpm);
    void brightnessChanged(quint8 brightness);
    void fanSpeedChanged(quint16 fan_rpm);
    void pumpDutyChanged(quint8 duty_percent);
    void fanDutyChanged(quint8 duty_percent);
    void fwInfoChanged(QString fwInfo);
    void versionChanged(QString version);
    void usbMessage(QJsonObject message);
    void rotationOffsetChanged(int rotation);
    void error(QJsonObject response);
    void imageTransfered(QImage frame);

public slots:
    void initialize();
    void startMonitoringFramerate();
    void stopMonitoringFramerate();
    void setContent(QQuickItem* content, quint32 frame_delay = 50);
    void clearContentItem();
    void setRotationOffset(int rotation);
    void setBrightness(quint8 brightness);
    void setFanDuty(quint8 duty);
    void setPumpDuty(quint8 duty); // flat
    void setImage(QString filepath, quint8 index = 0, bool applyAfterSet = true);
    void setImage(QImage image, quint8 index = 0, bool applyAfterSet = true);
    void sendStatusRequest();
    void sendHex(QString hex_data, bool pad = true);
    void moveToBucket( int bucket = 0);
    void setNZXTMonitor();
    void setBuiltIn(quint8 index);
    void blankScreen();

protected slots:
    void receivedControlResponse();
    void imageReady();
    void prepareNextFrame();
    void updateFrameRate();

protected:
#ifdef DEVELOPER_MODE
    void printQueryBucket(QByteArray & data);
    void printConfigBucket(QByteArray & data);
    void printSetupBucket(QByteArray & data);
    void printWriteBucket(QByteArray & data);
    void printSwitchBucket(QByteArray & data);
    void printFWRequest(QByteArray & data);
    void printStatusRequest(QByteArray & data);
    void parseResponseMessage(QByteArray& data);
#endif
    void parseFWVersion(QByteArray& data);
    void parseStatus(QByteArray& data);
    void parseDeleteBucket(QByteArray& data);

    // Control messages
    void sendFWRequest();
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
    QQuickItem*         mContent;
    QSharedPointer<QQuickItemGrabResult> mResult;
    short               mBufferIndex; // buffer index
    short               mImageIndex; // bucket id
    QTimer              mMeasure;
    short               mFrames;
    quint32             mFrameDelay;
    qreal               mFPS;
    QTimer              mDelayTimer;

};

#endif // KRAKENZDRIVER_H
