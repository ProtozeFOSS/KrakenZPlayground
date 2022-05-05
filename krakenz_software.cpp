#include "krakenz_software.h"
#include <QRandomGenerator>
#include <QTime>

KrakenZSoftware::KrakenZSoftware(QObject* parent)
    : KrakenZInterface{parent} , mBrightness{50}, mPumpDuty{50}, mFanDuty{50}, mFanSpeed{1050}, mLiquidTemp{30.2}, mPumpSpeed{1400},
      mFPS{0}, mRotationOffset{0}, mFrames{0}, mFanDelta{0}, mPumpDelta{0}, mWaterDelta{0},
      mPDutyTarget{50}, mFDutyTarget{50}, mFanTarget{1050}, mPumpTarget{1400}, mWaterTarget{30.2}
{
    // setup the update timer
    mStatusTimer.setInterval(345);
    connect(&mStatusTimer, &QTimer::timeout, this, &KrakenZSoftware::statusUpdate);
    mStatusTimer.setSingleShot(false);
    mMeasure.setInterval(1000);
    mMeasure.setSingleShot(false);
    connect(&mMeasure, &QTimer::timeout, this, &KrakenZSoftware::updateFramerate);
    recalculateWaterTemp();
}
bool KrakenZSoftware::initialize(bool &)
{
    mStatusTimer.start();
    qDebug() << version() << "initialized";
    return true;
}

bool KrakenZSoftware::initialized()
{
    return true;
}

bool KrakenZSoftware::found()
{
    return true;
}


double KrakenZSoftware::getPumpJitter()
{
    auto jitter(QRandomGenerator::global()->generate() % 48);
    double percent(0);
    switch(jitter){
        case 1:
        case 2:
        case 3: {
            percent = (jitter * .0015);
            break;
        }
        case 17: {
            percent = -.0052;
            break;
        }
        case 28: {
            percent = -.00235;
            break;
        }
        case 22: {
            percent = .0064;
            break;
        }
        case 12: {
            percent = -.0025;
            break;
        }
        case 0:
        default:;
    }
    return percent;
}

double KrakenZSoftware::getFanJitter()
{
    auto jitter{QRandomGenerator::global()->generate() % 12};
    double percent{0};
    switch(jitter){
        case 1:
        case 2:
        case 3: {
            percent = (jitter * .0035);
            break;
        }
        case 4: {
            percent = -.00875;
            break;
        }
        case 5: {
            percent = .0125;
            break;
        }
        case 6: {
            percent = -.00675;
            break;
        }
        case 0:
        default:;
    }
    return percent;
}


double KrakenZSoftware::getLiquidJitter()
{
    auto jitter{QRandomGenerator::global()->generate() % 100};
    double percent{0};
    if(jitter >= 98) {
        percent = .01 * (100 - jitter);
    } else if (jitter <= 2) {
        percent = -.01 * (100 - jitter);
    }
    return percent;
}


void KrakenZSoftware::recalculateWaterTemp()
{
    mWaterTarget = 32.25 - ((mFDutyTarget + mPDutyTarget) * .00865);
    mWaterDelta = (mWaterTarget - mLiquidTemp)/25;
}

// Slots
void KrakenZSoftware::setBrightness(quint8 value)
{
    if(value <= 100 && mBrightness != value) {
        mBrightness = value;
        emit brightnessChanged(value);
    }
}

void KrakenZSoftware::setBuiltIn(quint8)
{
    stopMonitoringFramerate();
}

void KrakenZSoftware::setFanDuty(quint8 value)
{
    if(value <= 100 && mFanDuty != value) {
        mFDutyTarget = value;
        mFanTarget = (21 * value);
        mFanDelta = (mFanTarget - mFanSpeed)/14;
        recalculateWaterTemp();
    }
}

void KrakenZSoftware::setImage(QImage, quint8, bool)
{
    ++mFrames;
    if(!mMeasure.isActive()) {
        mMeasure.start();
    }
}

void KrakenZSoftware::setPumpDuty(quint8 value)
{
    if(value <= 100 && mPumpDuty != value) {
        mPDutyTarget = value;
        mPumpTarget = (27.6 * value);
        mPumpDelta = (mPumpTarget - mPumpSpeed)/16;
        recalculateWaterTemp();
    }
}

void KrakenZSoftware::setNZXTMonitor()
{
    stopMonitoringFramerate();
}



void KrakenZSoftware::setRotationOffset(int offset)
{
    if(mRotationOffset != offset) {
        mRotationOffset = offset;
        emit rotationOffsetChanged(offset);
    }
}

void KrakenZSoftware::setScreenOrientation(Qt::ScreenOrientation orientation)
{
    auto rotation{0};
    switch(orientation) {
        case Qt::PrimaryOrientation:
        case Qt::LandscapeOrientation:
            rotation = 0;
            break;
        case Qt::PortraitOrientation:
            rotation = 90;
            break;
        case Qt::InvertedLandscapeOrientation:
            rotation = 180;
            break;
        case Qt::InvertedPortraitOrientation:
            rotation = 270;
            break;
        default:;
    }
    setRotationOffset(rotation);
}


void KrakenZSoftware::startMonitoringFramerate()
{
    mFrames = 0;
    mFPS = 0;
    emit fpsChanged(mFPS);
    mMeasure.start();
}

void KrakenZSoftware::statusUpdate()
{
    if(mPDutyTarget != mPumpDuty) {
        mPumpDuty = mPDutyTarget;
        emit pumpDutyChanged(mPumpDuty);
    }
    if(mFDutyTarget != mFanDuty) {
        mFanDuty = mFDutyTarget;
        emit fanDutyChanged(mFanDuty);
    }
    auto nextPumpSpeed{mPumpSpeed};
    if(mPumpTarget != nextPumpSpeed) {
        nextPumpSpeed = qAbs(mPumpTarget - mPumpSpeed) < qAbs(mPumpDelta) ? mPumpTarget: mPumpSpeed + mPumpDelta + (mPumpDelta * getPumpJitter());
        if(nextPumpSpeed >= 2800) {
            nextPumpSpeed = (27.6 * mPumpDuty);
        }
    }else {
        nextPumpSpeed += nextPumpSpeed * getPumpJitter();
    }
    if(nextPumpSpeed != mPumpSpeed) {
        mPumpSpeed = nextPumpSpeed;
        emit pumpSpeedChanged(nextPumpSpeed);
    }

    auto nextFanSpeed{mFanSpeed};
    if(mFanTarget != nextFanSpeed) {
        nextFanSpeed = qAbs(mFanTarget - mFanSpeed) < qAbs(mFanDelta) ? mFanTarget: mFanSpeed + mFanDelta + (mFanDelta * getFanJitter());
        if(nextFanSpeed >= 2100) {
            nextFanSpeed = mFanTarget = (20.9 * mFanDuty);
        }
        emit fanSpeedChanged(mFanSpeed);
    }else {
        nextFanSpeed += nextFanSpeed * getFanJitter();
    }
    if(nextFanSpeed != mFanSpeed) {
        mFanSpeed = nextFanSpeed;
        emit fanSpeedChanged(nextFanSpeed);
    }
}


void KrakenZSoftware::stopMonitoringFramerate()
{
    mMeasure.stop();
    mFrames = 0;
    mFPS = 0;
    emit fpsChanged(mFPS);
}

void KrakenZSoftware::updateFramerate()
{
    auto nextWaterTemp{qAbs(mWaterTarget - mLiquidTemp) < qAbs(mWaterDelta) ? mWaterTarget: mLiquidTemp + mWaterDelta};
    if(nextWaterTemp != mLiquidTemp) {
        mLiquidTemp = nextWaterTemp;
    }else {
        mLiquidTemp += (mLiquidTemp * getLiquidJitter());
    }
    emit liquidTemperatureChanged(mLiquidTemp);
    mFPS += (mFrames *.954);
    mFrames = 0;
    mFPS = (mFPS/2);
    emit fpsChanged(mFPS);
}
