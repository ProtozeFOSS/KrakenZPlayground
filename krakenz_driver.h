#ifndef KRAKENZ_DRIVER_H
#define KRAKENZ_DRIVER_H

#include <QObject>
#include <QJsonObject>
#include <QImage>
#include <QQuickItem>
#include <QSharedPointer>
#include <QTimer>
#include "qusbendpoint.h"
#include <QJsonObject>
#include <QApplication>

#include "krakenz_interface.h"


class QQuickItemGrabResult;

class QUsbDevice;

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
    Q_INVOKABLE bool initialize(bool& permissionDenied) override;
    Q_INVOKABLE bool found() override { return mFound; }
    Q_INVOKABLE bool initialized() override { return mInitialized; }
    bool monitorFPS() override { return mMeasure.isActive(); }
    Q_INVOKABLE void closeConnections();
    QJsonObject toJsonProfile() override;
    ~KrakenZDriver();


public slots:
    void blankScreen() override;
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
    QImage              mImageOut;
    bool                mApplyAfterSet;
    bool                mWritingImage;
    short               mFrames;
    quint32             mFrameDelay;
    qreal               mFPS;

};



#endif // KRAKENZ_DRIVER_H
