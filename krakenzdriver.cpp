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


KrakenZDriver::KrakenZDriver(QObject *parent, quint16 VID, quint16 PID)
    : QObject(parent), mLCDCTL(nullptr), mLiquidTemp(0), mFanSpeed(0), mPumpSpeed(0), mBrightness(80), mRotationOffset(0),
      mCMD(NO_TARGET), mSub(NO_TARGET), mBlocksToWrite(0), mBlocksWritten(0), mContent(nullptr), mBufferIndex(-1), mImageIndex(0),
      mImageTransferState(SELECT_TARGET), mFrames(0), mFrameDelay(0), mFPS(0)
{
    mMeasure.setInterval(1000);
    mMeasure.setSingleShot(false);
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

    // Attempt to find the Kraken device targets
      QUsb qusb;
//      qusb.setLogLevel(QUsb::logDebugAll);
//      qusb.xusb(id, lcdConfig);
    auto devices = qusb.devices();

//    for(const auto& deviceID: qAsConst(devices))
//    {
//        if(deviceID.vid == VID && deviceID.pid == PID && deviceID.endpoints.size() > 0){
//            qDebug() << "Kraken Found";
//            qDebug() << deviceID;
//            for(const auto& configDesc: qAsConst(deviceID.configurations)){
//                qDebug() << configDesc;
//            }
//            for(const auto& endpointDesc: qAsConst(deviceID.endpoints)){
//                qDebug() << endpointDesc;
//            }

//        }
//    }
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
    mDelayTimer.stop();
    mContent = nullptr;
    sendWriteFinishBucket(mImageIndex);
    mLCDCTL->waitForBytesWritten(1000);
    // mImageIndex ^= 1;
    mMeasure.stop();
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
    mMeasure.start();
    mImageIndex ^= 1;
    ++mFrames;
    if(mContent){
        mResult = mContent->grabToImage(QSize(320,320));
        connect(mResult.data(), &QQuickItemGrabResult::ready, this, &KrakenZDriver::imageReady);
    }else {
        mDelayTimer.stop();
    }
}

void KrakenZDriver::setFanDuty(quint8 duty)
{
    if(duty <= 100 && mFanDuty != duty){
        sendFanDuty(duty);
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

void KrakenZDriver::setPumpDuty(quint8 duty)
{
    if(duty >= 20 && duty <= 100 && mPumpDuty != duty){
        sendPumpDuty(duty);
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
    emit imageTransfered(image);
    QTransform rotation;
    rotation.rotate(mRotationOffset);
    image = image.transformed(rotation);
    if(index == mImageIndex){
        sendSwitchBucket(1,1);
    }
    mLCDCTL->waitForBytesWritten(1000);
    sendQueryBucket(index);
    mLCDCTL->waitForBytesWritten(1000);
    sendDeleteBucket(index);
    mLCDCTL->waitForBytesWritten(1000);
    auto byteCount = image.sizeInBytes();
    sendSetupBucket(index, index + 1, calculateMemoryStart(index), byteCount/1024);
    mLCDCTL->waitForBytesWritten(1000);
    sendWriteStartBucket(index);
    mLCDCTL->waitForBytesWritten(1000);
    sendBulkDataInfo(2, byteCount);
    mLCDDATA->waitForBytesWritten(2000);
    mLCDDATA->write(reinterpret_cast<const char*>(image.constBits()), byteCount);
    mLCDDATA->waitForBytesWritten(5000);
    sendWriteFinishBucket(index);
    mLCDCTL->waitForBytesWritten(1000);
    if(applyAfterSet)
    {
        sendSwitchBucket(index);
        mImageIndex = index;
        mLCDCTL->waitForBytesWritten(1000);
    }
}

void KrakenZDriver::setContent(QQuickItem* content, quint32 frame_delay)
{
    if(content != mContent){
        mDelayTimer.setInterval(frame_delay);
        mFrameDelay = frame_delay;
        mContent = content;
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

void KrakenZDriver::imageReady()
{
    auto frame = mResult->image();
    if(frame.isNull()){
        return;
    }
    frame.convertTo(QImage::Format_RGBA8888);
    QTransform rotation;
    rotation.rotate(mRotationOffset);
    frame = frame.transformed(rotation);
    sendSwitchBucket(mImageIndex ^ 1);
    mLCDCTL->waitForBytesWritten(1000);
    sendQueryBucket(mImageIndex);
    mLCDCTL->waitForBytesWritten(1000);
    sendDeleteBucket(mImageIndex);
    mLCDCTL->waitForBytesWritten(1000);
    auto byteCount = frame.sizeInBytes();
    sendSetupBucket(mImageIndex, mImageIndex + 1, calculateMemoryStart(mImageIndex) , byteCount/1024);
    mLCDCTL->waitForBytesWritten(1000);
    sendWriteStartBucket(mImageIndex);
    mLCDCTL->waitForBytesWritten(1000);
    sendBulkDataInfo(2, byteCount);
    mLCDDATA->waitForBytesWritten(2000);
    mLCDDATA->write(reinterpret_cast<const char*>(frame.constBits()), byteCount);
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
    mLCDDATA->write(bulk_info,512);
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

void KrakenZDriver::sendFanDuty(quint8 duty)
{
    QByteArray request_set_duty;
    request_set_duty.fill(0x0, _WRITE_LENGTH);
    request_set_duty[0] = SET_DUTY;
    request_set_duty[1] = 0x02; // FAN ADDRESS
    request_set_duty[4] = 0; // Critical Temp
    request_set_duty[5] = duty; // duty
    request_set_duty[6] = 59; // Critical Temp
    request_set_duty[7] = duty; // duty
    mLCDCTL->write(request_set_duty);
}

void KrakenZDriver::sendPumpDuty(quint8 duty)
{
    QByteArray request_set_duty;
    request_set_duty.fill(0x0, _WRITE_LENGTH);
    request_set_duty[0] = SET_DUTY;
    request_set_duty[1] = 0x01; // PUMP ADDRESS
    request_set_duty[4] = 0; // Critical Temp
    request_set_duty[5] = duty; // duty
    request_set_duty[6] = 59; // Critical Temp
    request_set_duty[7] = duty; // duty
    mLCDCTL->write(request_set_duty);
}

void KrakenZDriver::sendSetSpeedProfile(quint8 channel, quint8 profile, quint16 speed, bool direction)
{
    //TODO: Add set speed profile
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
   auto CMD = quint32(data[0]);
//   CMD += quint32(data[1]);
//   CMD += quint32(data[2]);
//   CMD += quint32(data[3]);
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
