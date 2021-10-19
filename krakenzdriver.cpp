#include "krakenzdriver.h"
#include "qusb.h"
#include "qusbdevice.h"
#include <QImage>
#include <QMetaEnum>
#include <QBuffer>
#include <QFile>
#include <QImageReader>
#include <QQuickItemGrabResult>
#include <QGuiApplication>
//#include <qusbcompositedevice.h>
const int _WRITE_LENGTH = 64;
const int _WRITE_BULK_LENGTH = 512;
const quint16 _MAX_MEMORY_SLOTS = 53255;
const quint16 _SLOTS_PER_INDEX = 36865;

constexpr char ID_TIMESTAMP[] = "TIME";
constexpr char ID_TYPE[] = "TYPE";
constexpr char ID_TARGET[] = "TARGET";
constexpr char ID_SESSION[] = "SESSION";
constexpr char ID_BUCKET[] = "BUCKET";
constexpr char ID_ASSET[] = "ASSET";
constexpr char ID_MODE[] = "MODE";
constexpr char ID_RAW[] = "RAW";
constexpr char ID_VALID[] = "VALID";
constexpr char ID_RECV[] = "RECEIVED";
constexpr char ID_MEMORY_START[] = "MEMORY_START";
constexpr char ID_MEMORY_SLOTS[] = "MEMORY_SLOTS";

constexpr quint8 CRITICAL_TEMP = 59;

KrakenZDriver::KrakenZDriver(QObject *parent, quint16 VID, quint16 PID)
    : QObject(parent), mFound(false), mKrakenDevice(nullptr), mLCDDATA(nullptr), mLCDCTL(nullptr), mLCDIN(nullptr), mLiquidTemp(0), mFanSpeed(0), mPumpSpeed(0), mBrightness(80), mRotationOffset(0),
      mCMD(NO_TARGET), mSub(NO_TARGET), mBlocksToWrite(0), mBlocksWritten(0), mContent(nullptr), mBufferIndex(-1), mImageIndex(0),
      mImageTransferState(SELECT_TARGET), mFrames(0), mFrameDelay(0), mFPS(0)
{
    mMeasure.setInterval(1000);
    mMeasure.setSingleShot(false);
    mMeasure.setTimerType(Qt::PreciseTimer);
    connect(&mMeasure, &QTimer::timeout, this, &KrakenZDriver::updateFrameRate);
    mDelayTimer.setTimerType(Qt::PreciseTimer);
    mDelayTimer.setSingleShot(true);
    mDelayTimer.setInterval(50);
    connect(&mDelayTimer, &QTimer::timeout, this, &KrakenZDriver::prepareNextFrame);
    QUsb::Config lcdConfig;
    QUsb::Id id;
    id.vid = VID;
    id.pid = PID;
    id.bus = 1;
    id.port = 13;
    mKrakenDevice = new QUsbDevice(this);
    mKrakenDevice->setId(id);
    mKrakenDevice->setConfig(lcdConfig);
    if(mKrakenDevice->open() ==  QUsb::statusOK)
    {
        mLCDDATA = new QUsbEndpoint(mKrakenDevice, QUsbEndpoint::bulkEndpoint, 2);
        if(!mLCDDATA->open(QIODevice::WriteOnly)){
            qDebug() << "Error opening Bulk write endpoint: " <<  mLCDDATA->errorString();
            qDebug() << id << mKrakenDevice->config();
        }
        mLCDCTL = new QUsbEndpoint(mKrakenDevice, QUsbEndpoint::interruptEndpoint, 1); // Write
        mLCDIN = new QUsbEndpoint(mKrakenDevice, QUsbEndpoint::interruptEndpoint, 129); // Read
        connect(mLCDIN, &QUsbEndpoint::readyRead, this, &KrakenZDriver::receivedControlResponse);
        mLCDIN->setPolling(true);
        if(!mLCDCTL->open(QIODevice::WriteOnly) ||  !mLCDIN->open(QIODevice::ReadOnly)){
            qDebug() << "Failed to open control endpoints: " << mLCDIN->errorString() << " : " << mLCDCTL->errorString();
            qDebug() << id << mKrakenDevice->config();
        } else {
            sendStatusRequest();
            if(mLCDDATA->isOpen()){
                mFound = true;
            }
        }
    }else{
        qDebug() << "Failed to open raw usb Kraken device:";
        qDebug() << id << mKrakenDevice->config();
    }

}

void KrakenZDriver::blankScreen()
{
    sendSwitchBucket(0,0);
}


quint16 KrakenZDriver::calculateMemoryStart(quint8 index)
{
    return index * 800;
}

void KrakenZDriver::clearContentItem()
{
    if(mContent){
        mContent = nullptr;
        mDelayTimer.stop();
        mMeasure.stop();
        sendSwitchBucket(mImageIndex);
        mLCDCTL->waitForBytesWritten(1000);
        mFPS = 0;
        mFrames = 0;
        emit fpsChanged(mFPS);
    }
}

void KrakenZDriver::clearQueuedWrite()
{
    mLCDIN->readAll();    
    mSub = mCMD = NO_TARGET;
}

void KrakenZDriver::moveToBucket(int bucket)
{
    sendSwitchBucket(bucket);
    mImageIndex = bucket;
}

void KrakenZDriver::prepareNextFrame()
{
    sendWriteFinishBucket(mImageIndex);
    mLCDCTL->waitForBytesWritten(1000);
    mImageIndex ^= 1;
    ++mFrames;
    if(mContent){
        mResult = mContent->grabToImage(QSize(320,320));
        connect(mResult.data(), &QQuickItemGrabResult::ready, this, &KrakenZDriver::imageReady);
    }
}


void KrakenZDriver::setBrightness(quint8 brightness)
{
    if(brightness <= 100 && mBrightness != brightness){
        sendBrightness(brightness);
        mBrightness = brightness;
        emit brightnessChanged(brightness);
    }
}


void KrakenZDriver::setRotationOffset(int rotation)
{
    if(mRotationOffset != rotation){
        mRotationOffset = rotation;
        emit rotationOffsetChanged(mRotationOffset);
    }
}

void KrakenZDriver::setImage(QString filepath, quint8 index,bool applyAfterSet)
{
    QImage image;
    filepath = filepath.replace("file:///","");
    if(image.load(filepath)){
        setImage(image, index, applyAfterSet);
    }else {
        qDebug() << "Failed to load image";
    }
}

void KrakenZDriver::setImage(QImage image, quint8 index, bool applyAfterSet)
{
    if(image.format() != QImage::Format_RGBA8888){
        image.convertTo(QImage::Format_RGBA8888);
    }
    if(image.width() != 320 || image.height() != 320){
        image = image.scaled(320,320);
    }
    QTransform rotation;
    rotation.rotate(mRotationOffset);
    auto image_out = image.transformed(rotation);
    if(index == mImageIndex){
        sendSwitchBucket(1,1);
    }
    mLCDCTL->waitForBytesWritten(1000);
    sendQueryBucket(index);
    mLCDCTL->waitForBytesWritten(1000);
    sendDeleteBucket(index);
    mLCDCTL->waitForBytesWritten(1000);
    auto byteCount = image_out.sizeInBytes();
    sendSetupBucket(index, index + 1, calculateMemoryStart(index), byteCount/1024);
    mLCDCTL->waitForBytesWritten(1000);
    sendWriteStartBucket(index);
    mLCDCTL->waitForBytesWritten(1000);
    sendBulkDataInfo(2, byteCount);
    mLCDDATA->waitForBytesWritten(2000);
    mLCDDATA->write(reinterpret_cast<const char*>(image_out.constBits()), byteCount);
    mLCDDATA->waitForBytesWritten(5000);
    sendWriteFinishBucket(index);
    mLCDCTL->waitForBytesWritten(1000);
    ++mFrames;
    if(applyAfterSet)
    {
        sendSwitchBucket(index);
        mImageIndex = index;
        mLCDCTL->waitForBytesWritten(1000);
        emit imageTransfered(image);
    }
}

void KrakenZDriver::setContent(QQuickItem* content, quint32 frame_delay)
{
    if(content != mContent){
        mDelayTimer.setInterval(frame_delay);
        mFrameDelay = frame_delay;
        mContent = content;
        mFrames = 0;
        mFPS = 0;
        mMeasure.start(1000);
        mResult = content->grabToImage(QSize(320,320));
        connect(mResult.data(), &QQuickItemGrabResult::ready, this, &KrakenZDriver::imageReady);
        emit contentChanged(content);
    }
}

void KrakenZDriver::setNZXTMonitor()
{
    sendSwitchBucket(0,2);
}

void KrakenZDriver::setBuiltIn(quint8 index)
{
    sendSwitchBucket(index,1);
}

void KrakenZDriver::startMonitoringFramerate()
{
    mFrames = 0;
    mFPS = 0;
    emit fpsChanged(mFPS);
    mMeasure.start(1000);
}

void KrakenZDriver::stopMonitoringFramerate()
{
    mMeasure.stop();
    mFrames = 0;
    mFPS = 0;
    emit fpsChanged(mFPS);
}

void KrakenZDriver::imageReady()
{
    auto frame = mResult->image();
    if(frame.isNull()){
        return;
    }
    frame.convertTo(QImage::Format_RGBA8888);
    QTransform rotation;
    rotation.rotate(mRotationOffset);
    auto frame_out = frame.transformed(rotation);
    sendSwitchBucket(mImageIndex ^ 1);
    mLCDCTL->waitForBytesWritten(1000);
    sendQueryBucket(mImageIndex);
    mLCDCTL->waitForBytesWritten(1000);
    sendDeleteBucket(mImageIndex);
    mLCDCTL->waitForBytesWritten(1000);
    auto byteCount = frame_out.sizeInBytes();
    sendSetupBucket(mImageIndex, mImageIndex + 1, calculateMemoryStart(mImageIndex) , byteCount/1024);
    mLCDCTL->waitForBytesWritten(1000);
    sendWriteStartBucket(mImageIndex);
    mLCDCTL->waitForBytesWritten(1000);
    sendBulkDataInfo(2, byteCount);
    mLCDDATA->waitForBytesWritten(2000);
    mLCDDATA->write(reinterpret_cast<const char*>(frame_out.constBits()), byteCount);
    mLCDDATA->waitForBytesWritten(5000);
    mDelayTimer.start(mFrameDelay);
    emit imageTransfered(frame);
}

void KrakenZDriver::updateFrameRate()
{
    mFPS += mFrames;
    mFrames = 0;
    mFPS = (mFPS/2);
    emit fpsChanged(mFPS);
}


void KrakenZDriver::parseDeleteBucket(QByteArray &data)
{

    qDebug() << "magic_1:" << data.left(2).toHex() << ",";
    qDebug() << "magic_2:" << data.mid(2,12).toHex() << ",";
    qDebug() << "bucket_id:" << data.mid(14,1).toHex() << ",";
    qDebug() << "asset_id:" << data.mid(15, 1).toHex()  << ",";
    qDebug() << "mode:" << data.mid(16, 1).toHex()  << ",";
    qDebug() << "start_memory_slot:" << data.mid(17, 2).toHex()  << ",";
    qDebug() << "number_of_memory_slot:" << data.mid(19, 2).toHex();
}

void KrakenZDriver::parseFWVersion(QByteArray &data)
{
    if(data.size() >= 20)
    {
        QString fw("0.0.0");
        fw[0] = QString::number(data.at(17)).at(0);fw[2]=QString::number(data.at(18)).at(0);fw[4]= QString::number(data.at(19)).at(0);
        qDebug() << "KRAKEN FW Version: v" << fw;
        mVersion = fw;
        emit fwInfoChanged(mVersion);
    }
}

void KrakenZDriver::parseStatus(QByteArray &data)
{
    if(data.size() >= 30)
    {
        //qDebug() << "Status: " << data.toHex();
        auto temp(qreal(float(data[15]) + float(data[16]) / 10));
        if(temp != mLiquidTemp){
            mLiquidTemp = temp;
            emit liquidTemperatureChanged(mLiquidTemp);
        }
        QByteArray speed_data;
        speed_data.reserve(2);
        speed_data[0] = data[18];
        speed_data[1] = data[17];
        auto converted(false);
        quint16 pump_speed = speed_data.toHex().toUInt(&converted, 16);
        if(pump_speed != mPumpSpeed){
            mPumpSpeed = pump_speed;
            emit pumpSpeedChanged(mPumpSpeed);
        }
        auto pump_duty((quint8(data[19])));
        if(pump_duty != mPumpDuty){
            mPumpDuty = pump_duty;
            emit pumpDutyChanged(mPumpDuty);
        }

        speed_data[0] = data[24];
        speed_data[1] = data[23];
        quint16 fan_speed(speed_data.toHex().toUInt(&converted,16));
        if(fan_speed != mFanSpeed){
            mFanSpeed = fan_speed;
            emit fanSpeedChanged(mFanSpeed);
        }
        auto fan_duty((quint8(data[25])));
        if(fan_duty != mFanDuty){
            mFanDuty = fan_duty;
            emit fanDutyChanged(mFanDuty);
        }
        qDebug() << "Liquid temperature " << mLiquidTemp << "°C";
        qDebug() << "Pump speed " << mPumpSpeed  << "rpm";
        qDebug() << "Pump duty " << mPumpDuty << "%";
        qDebug() << "Fan speed " << mFanSpeed << "rpm";
        qDebug() << "Fan duty " << mFanDuty << "%";
#ifdef DEVELOPER_MODE
        QJsonObject response;
        auto CMD = quint32(data[0]);;
        response.insert(ID_TIMESTAMP, QTime::currentTime().toString("h:mm:ss:zzz A"));
        response.insert(ID_TARGET, QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(CMD));
        response.insert(ID_TYPE, QString(data.left(2).toHex()));
        response.insert(ID_SESSION, QString(data.mid(2,12).toHex()));
        response.insert(ID_BUCKET, QString(data.mid(15,2).toHex()));
        response.insert(ID_ASSET, QString::number(quint8(data[19])));
        response.insert(ID_MODE, QString(data.mid(20,3).toHex()));
        response.insert(ID_MEMORY_START, QString(data.mid(17,2).toHex()));
        response.insert(ID_MEMORY_SLOTS, QString(data.mid(23,2).toHex()));
        response.insert(ID_RAW, QString(data.left(22).toHex()));
        response.insert(ID_VALID, true);
        response.insert(ID_RECV, true);
        emit usbMessage(response);
#endif
        //emit recievedFWVersion(mVersion);
    }
}

void KrakenZDriver::printConfigBucket(QByteArray &data)
{
#ifdef DEVELOPER_MODE
    QJsonObject response;
    response.insert(ID_TIMESTAMP, QTime::currentTime().toString("h:mm:ss:zzz A"));
    response.insert(ID_TARGET, QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(mCMD));
    response.insert(ID_TYPE, QString(data.left(2).toHex()));
    response.insert(ID_SESSION, QString(data.mid(2,12).toHex()));
    response.insert(ID_BUCKET, QString(data.mid(14,1).toHex()));
    response.insert(ID_ASSET, QString(data.mid(15, 1).toHex()));
    response.insert(ID_MODE, QString(data.mid(16, 1).toHex()));
    response.insert(ID_MEMORY_START, QString(data.mid(17, 2).toHex()));
    response.insert(ID_MEMORY_SLOTS, QString(data.mid(19, 2).toHex()));
    response.insert(ID_RAW, QString(data.left(22).toHex()));
    response.insert(ID_VALID, true);
    response.insert(ID_RECV, false);
    emit usbMessage(response);
#endif
    qDebug() << "Message Sent @ " << QTime::currentTime().toString() << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
    qDebug() << "RAW: " << data.toHex();
    qDebug() << "target:" << QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(mCMD);
    qDebug() << "cmd_type:" << data.left(2).toHex() << ",";
    qDebug() << "magic_2:" << data.mid(2,12).toHex() << ",";
    qDebug() << "bucket_id:" << data.mid(14,1).toHex() << ",";
    qDebug() << "asset_id:" << data.mid(15, 1).toHex()  << ",";
    qDebug() << "mode:" << data.mid(16, 1).toHex()  << ",";
    qDebug() << "start_memory_slot:" << data.mid(17, 2).toHex()  << ",";
    qDebug() << "number_of_memory_slot:" << data.mid(19, 2).toHex();
    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
}

void KrakenZDriver::printFWRequest(QByteArray &data)
{
#ifdef DEVELOPER_MODE
    QJsonObject response;
    response.insert(ID_TIMESTAMP, QTime::currentTime().toString("h:mm:ss:zzz A"));
    response.insert(ID_TARGET, QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(mCMD));
    response.insert(ID_TYPE, QString(data.left(2).toHex()));
    response.insert(ID_RAW, QString(data.left(22).toHex()));
    response.insert(ID_BUCKET, "");
    response.insert(ID_ASSET, "");
    response.insert(ID_MODE, "");
    response.insert(ID_SESSION, "");
    response.insert(ID_VALID, true);
    response.insert(ID_RECV, false);
    emit usbMessage(response);
#endif
//    qDebug() << "Message Sent @ " << QTime::currentTime().toString() << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
//    qDebug() << "RAW: " << data.toHex();
//    qDebug() << "target:" << QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(mCMD);
//    qDebug() << "cmd_type:" << data.left(2).toHex() << ",";
//    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
}

void KrakenZDriver::sendQueuedWrite()
{
    // switch on the queued type and sub type
    if(quint8(mQueuedData[0]) == 0) {
        return;
    }
    auto CMD = WriteTarget(quint8(mQueuedData[0]));
    switch(CMD){
        case AIO_STATUS:{
            printStatusRequest(mQueuedData);
            break;
        }
        case QUERY_BUCKET:{
            printQueryBucket(mQueuedData);
            break;
        }
        case SETUP_BUCKET:{
            printSetupBucket(mQueuedData);
            break;
        }
        case WRITE_SETUP:{
            printWriteBucket(mQueuedData);
            break;
        }
        case SWITCH_BUCKET:{
            printSwitchBucket(mQueuedData);
            break;
        }
        default:{
            qDebug() << "Unhandled Case" << QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(CMD);
            printQueryBucket(mQueuedData);
            break;
        }
    }
    mLCDCTL->write(mQueuedData);
    mQueuedData.fill(0);
    mSub = mCMD = NO_TARGET;
}


void KrakenZDriver::printQueryBucket(QByteArray &data)
{
#ifdef DEVELOPER_MODE
    QJsonObject message;
    auto CMD = quint32(data[0]);
    message.insert(ID_TIMESTAMP, QTime::currentTime().toString("h:mm:ss:zzz A"));
    message.insert(ID_TARGET, QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(CMD));
    message.insert(ID_TYPE, QString(data.left(2).toHex()));
    message.insert(ID_BUCKET, QString::number(qint32(data[3])));
    message.insert(ID_ASSET, QString::number(qint32(data[5])));
    message.insert(ID_RAW, QString(data.left(22).toHex()));
    message.insert(ID_MODE, "");
    message.insert(ID_SESSION, "");
    message.insert(ID_MEMORY_START, "");
    message.insert(ID_MEMORY_SLOTS, "");
    message.insert(ID_VALID, true);
    message.insert(ID_RECV, false);
    emit usbMessage(message);
#endif
//    qDebug() << "Message Sent @ " << QTime::currentTime().toString() << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
//    qDebug() << "RAW: " << data.toHex();
//    qDebug() << "msg_type:" << QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(mCMD);
//    qDebug() << "cmd_type:" << data.left(2).toHex() << ",";
//    qDebug() << "bucket_id:" << qint32(data[3]) << ",";
//    qDebug() << "asset_id:" << qint32(data[5])  << ",";
//    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";

}

void KrakenZDriver::printSetupBucket(QByteArray &data)
{
    // Determine if its a Delete Bucket or Setup Bucket
#ifdef DEVELOPER_MODE
    QJsonObject message;
    message.insert(ID_TIMESTAMP, QTime::currentTime().toString("h:mm:ss:zzz A"));
    message.insert(ID_TARGET, QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(mCMD));
    message.insert(ID_TYPE, QString(data.left(2).toHex()));
    message.insert(ID_SESSION, QString(data.mid(2,12).toHex()));
    message.insert(ID_BUCKET, QString(data.mid(14,1).toHex()));
    message.insert(ID_ASSET, QString(data.mid(15, 1).toHex()));
    message.insert(ID_MODE, QString(data.mid(16, 1).toHex()));
    message.insert(ID_MEMORY_START, QString(data.mid(17, 2).toHex()));
    message.insert(ID_MEMORY_SLOTS, QString(data.mid(19, 2).toHex()));
    message.insert(ID_RAW, QString(data.left(22).toHex()));
    message.insert(ID_VALID, true);
    message.insert(ID_RECV, false);
    emit usbMessage(message);
#endif
//    qDebug() << "Message Sent @ " << QTime::currentTime().toString() << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
//    qDebug() << "RAW: " << data.toHex();
//    qDebug() << "target:" << QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(mCMD);
//    qDebug() << "cmd_type:" << data.left(2).toHex() << ",";
//    qDebug() << "magic_2:" << data.mid(2,12).toHex() << ",";
//    qDebug() << "bucket_id:" << data.mid(14,1).toHex() << ",";
//    qDebug() << "asset_id:" << data.mid(15, 1).toHex()  << ",";
//    qDebug() << "mode:" << data.mid(16, 1).toHex()  << ",";
//    qDebug() << "start_memory_slot:" << data.mid(17, 2).toHex()  << ",";
//    qDebug() << "number_of_memory_slot:" << data.mid(19, 2).toHex();
//    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
}

void KrakenZDriver::printWriteBucket(QByteArray &data)
{
    // check the sub type
    if(data.size() < 30) {
        return;
    }
    auto CMD = quint32(data[0]);
    auto sub = quint8(data[1]);

#ifdef DEVELOPER_MODE
    QJsonObject message;
    message.insert(ID_TIMESTAMP, QTime::currentTime().toString("h:mm:ss:zzz A"));
    message.insert(ID_TARGET, QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(CMD));
    message.insert(ID_TYPE, QString(data.left(2).toHex()));
    message.insert(ID_BUCKET, QString::number(data[3]));
    message.insert(ID_ASSET, QString::number(data[5]));
    message.insert(ID_RAW, QString(data.left(22).toHex()));
    message.insert(ID_SESSION, "");
    message.insert(ID_MODE, "");
    message.insert(ID_MEMORY_START, "");
    message.insert(ID_MEMORY_SLOTS, "");
    message.insert(ID_VALID, true);
    message.insert(ID_RECV, false);
    emit usbMessage(message);
#endif
//    qDebug() << "Message Sent @ " << QTime::currentTime().toString() << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
//    qDebug() << "RAW: " << data.toHex();
//    qDebug() << "msg_type: WRITE_BUCKET";
//    if(sub == WRITE_START){
//        qDebug() << "msg_sub: WRITE_START";
//    } else { // write finish
//        qDebug() << "msg_sub: WRITE_FINISH";
//    }
//    qDebug() << "cmd_type:" << data.left(2).toHex() << ",";
//    qDebug() << "bucket_id:" << qint32(data[3]) << ",";
//    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
}

void KrakenZDriver::printSwitchBucket(QByteArray &data)
{
#ifdef DEVELOPER_MODE
    QJsonObject message;
    message.insert(ID_TIMESTAMP, QTime::currentTime().toString("h:mm:ss:zzz A"));
    message.insert(ID_TARGET, QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(SWITCH_BUCKET));
    message.insert(ID_TYPE, "3801");
    message.insert(ID_SESSION, "");
    message.insert(ID_BUCKET, QString::number(data[4]));
    message.insert(ID_ASSET, "");
    message.insert(ID_MODE, QString(data.mid(2, 2).toHex()));
    message.insert(ID_MEMORY_START, "");
    message.insert(ID_MEMORY_SLOTS, "");
    message.insert(ID_RAW, QString(data.left(22).toHex()));
    message.insert(ID_VALID, true);
    message.insert(ID_RECV, false);
    emit usbMessage(message);
#endif
//    qDebug() << "Message Sent @ " << QTime::currentTime().toString() << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
//    qDebug() << "RAW: " << data.toHex();
//    qDebug() << "target:" << QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(SWITCH_BUCKET);
//    qDebug() << "cmd_type:" << data.left(2).toHex() << ",";
//    qDebug() << "bucket_id:" << qint32(data[4]) << ",";
//    qDebug() << "mode:" << QString(data.mid(2, 2).toHex());
//    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
}

void KrakenZDriver::printStatusRequest(QByteArray &data)
{
    QJsonObject message;
    message.insert(ID_TIMESTAMP, QTime::currentTime().toString("h:mm:ss:zzz A"));
    message.insert(ID_TARGET, QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(AIO_STATUS));
    message.insert(ID_TYPE, "7401");
    message.insert(ID_SESSION, "");
    message.insert(ID_BUCKET, QString::number(data[4]));
    message.insert(ID_ASSET, "");
    message.insert(ID_MODE, "");
    message.insert(ID_MEMORY_START, "");
    message.insert(ID_MEMORY_SLOTS, "");
    message.insert(ID_RAW, QString(data.left(22).toHex()));
    message.insert(ID_VALID, true);
    message.insert(ID_RECV, false);
    emit usbMessage(message);
//    qDebug() << "Message Sent @ " << QTime::currentTime().toString() << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
//    qDebug() << "RAW: " << data.toHex();
//    qDebug() << "target:" << QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(AIO_STATUS);
//    qDebug() << "cmd_type:" << data.left(2).toHex() << "";
//    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
}


void KrakenZDriver::parseResponseMessage(QByteArray &data)
{
    QJsonObject response;
    auto CMD = quint8(data[0]);
    response.insert(ID_TIMESTAMP, QTime::currentTime().toString("h:mm:ss:zzz A"));
    response.insert(ID_TARGET, QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(CMD));
    response.insert(ID_TYPE, QString(data.left(2).toHex()));
    response.insert(ID_SESSION, QString(data.mid(2,12).toHex()));
    response.insert(ID_BUCKET, QString(data.mid(14,1).toHex()));
    response.insert(ID_ASSET, QString(data.mid(15, 1).toHex()));
    response.insert(ID_MODE, QString(data.mid(16, 1).toHex()));
    response.insert(ID_MEMORY_START, QString(data.mid(17, 2).toHex()));
    response.insert(ID_MEMORY_SLOTS, QString(data.mid(19, 2).toHex()));
    response.insert(ID_RAW, QString(data.left(22).toHex()));
    response.insert(ID_VALID, true);
    response.insert(ID_RECV, true);
    emit usbMessage(response);
//    qDebug() << "Message Received @ " << QTime::currentTime().toString() << " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
//    qDebug() << "RAW: " << data.toHex();
//    qDebug() << "target:" << QMetaEnum::fromType<KrakenZDriver::WriteTarget>().valueToKey(mCMD);
//    qDebug() << "cmd_type:" << data.left(2).toHex() << ",";
//    qDebug() << "magic_2:" << data.mid(2,12).toHex() << ",";
//    qDebug() << "bucket_id:" << data.mid(14,1).toHex() << ",";
//    qDebug() << "asset_id:" << data.mid(15, 1).toHex()  << ",";
//    qDebug() << "mode:" << data.mid(16, 1).toHex()  << ",";
//    qDebug() << "start_memory_slot:" << data.mid(17, 2).toHex()  << ",";
//    qDebug() << "number_of_memory_slot:" << data.mid(19, 2).toHex();
//    qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n";
}



void KrakenZDriver::sendBulkDataInfo(quint8 mode, quint32 size)
{
    QByteArray bulk_info;
    bulk_info.fill(0, _WRITE_BULK_LENGTH);

    QString hexData(QStringLiteral("12fa01e8abcdef98765432100"));
    QString mode_out(QString::number(mode,10));
    hexData += mode_out;
    auto output_hex = hexData.toLatin1();
    bulk_info.replace(0, output_hex.size(), QByteArray::fromHex(output_hex));
    bulk_info[17] = 0x40;
    bulk_info[18] = 0x96;
    //qDebug() << "Bulk Info: " << bulk_info.left(20).toHex();
    mLCDDATA->write(bulk_info);
}

void KrakenZDriver::sendBrightness(quint8 brightness)
{
    QByteArray request_set_duty;
    request_set_duty.fill(0x0, _WRITE_LENGTH);
    request_set_duty[0] = QUERY_BUCKET;
    request_set_duty[1] = BRIGHTNESS;
    request_set_duty[2] = SET_BUCKET;
    request_set_duty[3] = brightness;
    mLCDCTL->write(request_set_duty);

}

void KrakenZDriver::setFanDuty(quint8 duty)
{
    if(duty >= 0 && duty <= 100){
        QByteArray set_set_fan;
        set_set_fan.fill(0x0, _WRITE_LENGTH);
        set_set_fan[0] = SET_DUTY;
        set_set_fan[1] = FAN_CHANNEL;
        if(duty != 0){
            set_set_fan[4] = duty;
            set_set_fan[5] = duty;
            set_set_fan[6] = duty;
            set_set_fan[7] = duty;
            set_set_fan[8] = duty;
            set_set_fan[9] = duty;
            set_set_fan[10] = duty;
            set_set_fan[11] = duty;
            set_set_fan[12] = duty;
            set_set_fan[13] = duty;
            set_set_fan[14] = duty;
            set_set_fan[15] = duty;
            set_set_fan[16] = duty;
            set_set_fan[17] = duty;
            set_set_fan[18] = duty;
            set_set_fan[19] = duty;
            set_set_fan[20] = duty;
            set_set_fan[21] = duty;
            set_set_fan[22] = duty;
            set_set_fan[23] = duty;
            set_set_fan[24] = duty;
            set_set_fan[25] = duty;
            set_set_fan[26] = duty;
            set_set_fan[27] = duty;
            set_set_fan[28] = duty;
            set_set_fan[29] = duty;
            set_set_fan[30] = duty;
            set_set_fan[31] = duty;
            set_set_fan[32] = duty;
            set_set_fan[33] = duty;
            set_set_fan[34] = duty;
            set_set_fan[35] = duty;
            set_set_fan[36] = duty;
            set_set_fan[37] = duty;
            set_set_fan[38] = duty;
            set_set_fan[39] = duty;
            set_set_fan[40] = duty;
            set_set_fan[41] = duty;
            set_set_fan[42] = duty;
            set_set_fan[43] = duty;
        }
        mLCDCTL->write(set_set_fan);
        mLCDCTL->waitForBytesWritten(1000);
    }
}

void KrakenZDriver::setPumpDuty(quint8 duty)
{
    if(duty >= 20 && duty <= 100){
        QByteArray set_pump;
        set_pump.fill(0x0, _WRITE_LENGTH);
        set_pump[0] = SET_DUTY;
        set_pump[1] = PUMP_CHANNEL;
        set_pump[4] = duty;
        set_pump[5] = duty;
        set_pump[6] = duty;
        set_pump[7] = duty;
        set_pump[8] = duty;
        set_pump[9] = duty;
        set_pump[10] = duty;
        set_pump[11] = duty;
        set_pump[12] = duty;
        set_pump[13] = duty;
        set_pump[14] = duty;
        set_pump[15] = duty;
        set_pump[16] = duty;
        set_pump[17] = duty;
        set_pump[18] = duty;
        set_pump[19] = duty;
        set_pump[20] = duty;
        set_pump[21] = duty;
        set_pump[22] = duty;
        set_pump[23] = duty;
        set_pump[24] = duty;
        set_pump[25] = duty;
        set_pump[26] = duty;
        set_pump[27] = duty;
        set_pump[28] = duty;
        set_pump[29] = duty;
        set_pump[30] = duty;
        set_pump[31] = duty;
        set_pump[32] = duty;
        set_pump[33] = duty;
        set_pump[34] = duty;
        set_pump[35] = duty;
        set_pump[36] = duty;
        set_pump[37] = duty;
        set_pump[38] = duty;
        set_pump[39] = duty;
        set_pump[40] = duty;
        set_pump[41] = duty;
        set_pump[42] = duty;
        set_pump[43] = duty;
        mLCDCTL->write(set_pump);
        mLCDCTL->waitForBytesWritten(1000);
    }
}


void generate_normalized_profile(QList<TempPoint> &normalized, const QList<TempPoint>& profile, quint8 max_first)
{
    if(profile.size() == 0){
        normalized.clear();
        return;
    }
    int c_index = 0;
    qDebug() << "Profile : ";
    QList<TempPoint> lower;
    int l_high_temp(max_first/2);
    int l_low_temp(l_high_temp);
    QList<TempPoint> upper;
    int u_low_temp(max_first/2+1);
    int u_high_temp(u_low_temp);
//    normalized.reserve(profile.size());
    auto length = profile.size();
    for(c_index = 0; c_index < length; ++c_index){
        auto tempPoint = profile.at(c_index);
        if(tempPoint.temp <= l_low_temp){ // lowest
            l_low_temp = tempPoint.temp;
            lower.prepend(tempPoint);
        }else if(tempPoint.temp < u_low_temp){ // in lower
            if(tempPoint.temp >= l_high_temp){ // highest of the lows
                lower.append(tempPoint);
                l_high_temp = tempPoint.temp;
            } else { // insert in lower
                // find insert point
                for(int i(lower.size()-1); i >= 0; --i ){
                    const auto cmp = lower.at(i);
                    if(i == 0){
                        lower.insert(0,tempPoint);
                        l_low_temp = tempPoint.temp;
                        break;
                    }else if(cmp.temp >= tempPoint.temp){
                        lower.insert(i,tempPoint);
                        break;
                    }
                }
            }
        }else if(tempPoint.temp >= u_high_temp){ // found highest
            upper.append(tempPoint);
            u_high_temp = tempPoint.temp;
        } else{ // insert in higher
            auto u_length = upper.size();
            for(int i(0); i < u_length; ++i ){
                const auto cmp = lower.at(i);
                if(cmp.temp <= tempPoint.temp){
                    upper.insert(i,tempPoint);
                    break;
                }
            }
        }
    }
    normalized = lower;
    normalized += upper;
    qDebug() << "Sorted By Temp: ";
    c_index = 0;
    for(auto const& point: normalized){
        qDebug("[%d] -> temp:%d , point:%d" , c_index++, point.temp, point.point);
    }
    // final pass makes sure there are not duplicate temps, if a duplicate is found, it will be set to the average of the next two points
    // if it is the last point, set it to an avg of max and current
    length = normalized.length();
    auto last_temp = normalized.at(0).temp;
    auto items_remain = length;
    for(c_index = 1; c_index < length; ++c_index){
        auto point = normalized.at(c_index);
        if(last_temp >= point.temp){
            if(c_index == length-1){
                point.temp = ((point.temp + max_first)/2) - 1;
            }else {
                point.temp = (point.temp + last_temp)/2 -1;
            }
            normalized[c_index] = point;
        }
        last_temp = point.temp;
        if(last_temp == max_first){
            items_remain = c_index+1;
        }
    }
    if(items_remain != normalized.size()){
        for(int size = normalized.size(); size > items_remain; --size){
            normalized.pop_back();
        }
    }
    auto last_point = normalized.at(0).point;
    length = normalized.size();
    items_remain = length;
    for(c_index = 1; c_index < length; ++c_index){
        auto point = normalized.at(c_index);
        if(last_point >= point.point){
            if(c_index == length-1){
                point.point = ((point.point + 100)/2) + 1;
            }else {
                auto& previous = normalized[c_index-1];
                previous.point = (point.point + last_point)/2 -1;
                point.point = last_point;
            }
            normalized[c_index] = point;
        }
        last_point = point.point;
        if(last_point == 100){
            items_remain = c_index+1;
        }
    }
    if(items_remain != normalized.size()){
        for(int size = normalized.size(); size > items_remain; --size){
            normalized.pop_back();
        }
    }
    qDebug() << "Normalized: ";
    c_index = 0;
    for(auto const& point: normalized){
        qDebug("[%d] -> temp:%d , point:%d" , c_index++, point.temp, point.point);
    }
}

void KrakenZDriver::sendSetDutyProfile(quint8 channel, const QList<TempPoint>& profile)
{
    QList<TempPoint> profile_out;
    QList<TempPoint> test;
    test.append(TempPoint(30,40));
    test.append(TempPoint(25,25));
    test.append(TempPoint(35,30));
    test.append(TempPoint(40,35));
    test.append(TempPoint(40,80));
    generate_normalized_profile(profile_out, test, 60);
    test.clear();
    test << TempPoint(30,40) << TempPoint(25,25) << TempPoint(35,30) << TempPoint(40,100);
    generate_normalized_profile(profile_out, test, 60);
    test.clear();
    test << TempPoint(30,40) << TempPoint(25,25) << TempPoint(35,100) << TempPoint(40,100);
    generate_normalized_profile(profile_out, test, 60);
    QByteArray request_set_duty;
//    request_set_duty.fill(0x0, _WRITE_LENGTH);
//    request_set_duty[0] = SET_DUTY;
//    request_set_duty[1] = channel;
//    request_set_duty[4] = 0;
//    request_set_duty[5] = duty; // duty
//    request_set_duty[6] = 59; // Critical Temp
//    request_set_duty[7] = duty; // duty
//    mLCDCTL->write(request_set_duty);
}


void KrakenZDriver::sendHex(QString hex_data, bool pad)
{
    if(pad && hex_data.size() < (_WRITE_LENGTH * 2)) // byte = 2 HEX nibbles
    {
        QString pad_str;
        pad_str.fill('0', (_WRITE_LENGTH *2 - hex_data.size()));
        hex_data.append(pad_str);
    }
    mLCDCTL->write(QByteArray::fromHex(hex_data.toLatin1()));
}

void KrakenZDriver::sendFWRequest()
{

    QByteArray request_fw;
    request_fw.fill(0x0, _WRITE_LENGTH - 2 );
    request_fw.prepend(0x01);
    request_fw.prepend(FW_VERSION);
    mLCDCTL->write(request_fw);
#ifdef DEVELOPER_MODE
        printFWRequest(request_fw);
#endif
}

void KrakenZDriver::sendDeleteBucket(quint8 index)
{
    // precondition check on index
    if(index > 15 || index < 0){
        return;
    }
    QByteArray delete_bucket;
    delete_bucket.fill(0x0, _WRITE_LENGTH);
    delete_bucket[0] = SETUP_BUCKET;
    delete_bucket[1] = DELETE_BUCKET;
    delete_bucket[2] = index;
    mCMD = SETUP_BUCKET;
    mSub = DELETE_BUCKET;
    mLCDCTL->write(delete_bucket);
#ifdef DEVELOPER_MODE
        printSetupBucket(delete_bucket);
#endif
}

void KrakenZDriver::sendWriteFinishBucket(quint8 index)
{
    if(index > 15 || index < 0){
        return;
    }
    QByteArray finish_write;
    finish_write.fill(0x0, _WRITE_LENGTH);
    finish_write[0] = WRITE_SETUP;
    finish_write[1] = WRITE_FINISH;
    finish_write[2] = index;
    mCMD = WRITE_SETUP;
    mSub = WRITE_FINISH;
    mLCDCTL->write(finish_write);
#ifdef DEVELOPER_MODE
    printWriteBucket(finish_write);
#endif
}

void KrakenZDriver::sendWriteStartBucket(quint8 index)
{
    if(index > 15 || index < 0){
        return;
    }
    QByteArray prepare_write;
    prepare_write.fill(0x0, _WRITE_LENGTH);
    prepare_write[0] = WRITE_SETUP;
    prepare_write[1] = WRITE_START;
    prepare_write[2] = index;
    mSub = WRITE_START;
    mLCDCTL->write(prepare_write);
#ifdef DEVELOPER_MODE
    printWriteBucket(prepare_write);
#endif
}

void KrakenZDriver::sendSetupBucket(quint8 index, quint8 id, quint16 memory_slot, quint16 memory_slot_count)
{
    QByteArray setup_bucket;
    setup_bucket.fill(0x0, _WRITE_LENGTH);
    setup_bucket[0] = SETUP_BUCKET;
    setup_bucket[1] = SET_BUCKET;
    setup_bucket[2] = index;
    setup_bucket[3] = id;
    setup_bucket[4] = quint8(memory_slot >> 8);
    setup_bucket[5] = quint8(memory_slot);
    setup_bucket[6] = quint8(memory_slot_count);
    setup_bucket[7] = quint8(memory_slot_count >> 8);
    setup_bucket[8] = 1;
    mCMD = SETUP_BUCKET;
    mSub = SET_BUCKET;
    mLCDCTL->write(setup_bucket);
#ifdef DEVELOPER_MODE
    printSetupBucket(setup_bucket);
#endif
}

void KrakenZDriver::sendSwitchBucket(quint8 index, quint8 mode)
{
    if(index > 15 || index < 0){
        return;
    }
    QByteArray switch_bucket;
    switch_bucket.fill(0x0, _WRITE_LENGTH);
    switch_bucket[0] = SWITCH_BUCKET;
    switch_bucket[1] = 0x1;
    switch_bucket[2] = mode;
    switch_bucket[3] = index;
    mCMD = SWITCH_BUCKET;
    mSub = NO_TARGET;
    mLCDCTL->write(switch_bucket);
#ifdef DEVELOPER_MODE
    printSwitchBucket(switch_bucket);
#endif
}

void KrakenZDriver::sendSwitchLiquidTempMode()
{
    sendSwitchBucket(0, 2);
}

void KrakenZDriver::sendStatusRequest()
{
    QByteArray request_status;
    request_status.fill(0x0, _WRITE_LENGTH);
    request_status[0] = AIO_STATUS;
    request_status[1] = 0x1;
    if(!mLCDCTL->isOpen()){
        mLCDCTL->open(QIODevice::WriteOnly);
    }
    mCMD = AIO_STATUS;
    mSub = WRITE_FINISH;
    mLCDCTL->write(request_status);
#ifdef DEVELOPER_MODE
    printStatusRequest(request_status);
#endif
}

void KrakenZDriver::sendQueryBucket(quint8 index, quint8 asset)
{
    // precondition check on index
    if(index > 15 || index < 0){
        return;
    }
    QByteArray query_bucket;
    query_bucket.fill(0x0, _WRITE_LENGTH);
    query_bucket[0] = QUERY_BUCKET; // QUERY
    query_bucket[1] = UNKNOWN_SUB; // 04 SUB?
    query_bucket[3] = index;
    query_bucket[5] = asset;
    mCMD = QUERY_BUCKET;
    mSub = UNKNOWN_SUB;
    mLCDCTL->write(query_bucket);
#ifdef DEVELOPER_MODE
            printQueryBucket(query_bucket);
#endif
}


void KrakenZDriver::receivedControlResponse()
{
   auto data = mLCDIN->readAll();
   if(data.size() == 0)
       return;
   auto CMD = quint8(data[0]);
   switch(CMD){
       case RESPONSE_VERSION:
       {
           parseFWVersion(data);
           break;
       }
       case RESPONSE_STATUS:
       {
           parseStatus(data);
           break;
       }
       default:
       {
           parseResponseMessage(data);
           break;
       }
   }
   if(quint8(mQueuedData[0]) != NO_TARGET){
       sendQueuedWrite();
   }else {
       mCMD = NO_TARGET;
   }
}


KrakenZDriver::~KrakenZDriver()
{
    mLCDCTL->close();
    mLCDDATA->close();
    mLCDIN->close();
    QObject::disconnect(mLCDCTL);
    QObject::disconnect(mLCDDATA);
    QObject::disconnect(mLCDIN);
    delete mLCDCTL;
    mLCDCTL = nullptr;
    delete mLCDDATA;
    mLCDDATA = nullptr;
    delete mLCDIN;
    mLCDIN = nullptr;
    delete mKrakenDevice;
    mCMD = NO_TARGET;
}
