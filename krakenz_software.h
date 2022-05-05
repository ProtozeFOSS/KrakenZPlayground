#ifndef KRAKENZ_SOFTWARE_H
#define KRAKENZ_SOFTWARE_H

#include "krakenz_interface.h"
#include <QTimer>

class KrakenZSoftware: public KrakenZInterface {
    Q_OBJECT
public:
    KrakenZSoftware(QObject* parent = nullptr);
    Q_INVOKABLE bool initialize(bool&) override;
    Q_INVOKABLE bool initialized() override;

    // Property getters
    bool found() override;
    quint8 brightness() override { return mBrightness; }
    quint8 fanDuty() override { return mFanDuty; }
    quint16 fanSpeed() override { return mFanSpeed; }
    qreal fps() override { return mFPS; }
    qreal liquidTemperature() override { return mLiquidTemp; }
    virtual int rotationOffset() override { return mRotationOffset; }
    quint8 pumpDuty() override { return mPumpDuty; }
    quint16 pumpSpeed() override { return mPumpSpeed; }
    QString version() override { return QStringLiteral("KrakenZ SW EMU v1.0.0"); }

    //  Profile load and save
//    void setJsonProfile(QJsonObject profile) override;
//    QJsonObject toJsonProfile() override;

public slots:
    // Consumer API
    void setBrightness(quint8 value) override;
    void setFanDuty(quint8 value) override;
    void setPumpDuty(quint8 duty) override;
    void setImage(QImage, quint8 = 0, bool = true) override;

    // void setMonitorFPS(bool monitor = true) override;
    void setBuiltIn(quint8) override;
    void setNZXTMonitor() override;
    void setRotationOffset(int offset) override;
    void setScreenOrientation(Qt::ScreenOrientation orientation) override;

    //    // Frame Rate monitoring
    void startMonitoringFramerate() override;
    void stopMonitoringFramerate() override;

protected slots:
    void  updateFramerate();
    void  statusUpdate(); // emulate getting a reply from the device

protected:
    quint8     mBrightness;
    quint8     mPumpDuty;
    quint8     mFanDuty;
    quint16    mFanSpeed;
    qreal      mLiquidTemp;
    quint16    mPumpSpeed;
    qreal      mFPS;
    QTimer     mStatusTimer; // triggers new values
    int        mRotationOffset; // lcd rotation offset
    quint8     mFrames; // frame counter
    QTimer     mMeasure; // measures fps

    // deltas for fan, pump, and water
    qint32     mFanDelta;
    qint32     mPumpDelta;
    qreal      mWaterDelta;

    // Targets, these will help shape the stopping point for changes
    quint8     mPDutyTarget;
    quint8     mFDutyTarget;
    quint16    mFanTarget;
    quint16    mPumpTarget;
    qreal      mWaterTarget;

    double     getFanJitter();
    double     getLiquidJitter();
    double     getPumpJitter();
    void       recalculateWaterTemp();

};


#endif // KRAKENZ_SOFTWARE_H
