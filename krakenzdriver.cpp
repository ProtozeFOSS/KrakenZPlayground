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
const int _WRITE_LENGTH = 64;
const int _WRITE_BULK_LENGTH = 512;
const quint16 _MAX_MEMORY_SLOTS = 53255;
const quint16 _SLOTS_PER_INDEX = 36865;

constexpr char ID_TIMESTAMP[] = "TIME";
constexpr char ID_TYPE[] = "TYPE";
constexpr char ID_MESSAGE[] = "MESSAGE";
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


#ifdef Q_OS_LINUX
constexpr char OPEN_DENIED_STR[] = "Failed to open root USB device \n Check if another process has control of the Kraken Z and UDEV rules are setup correctly";
#else
constexpr char OPEN_DENIED_STR[] = "Failed to open root USB device \n Check if another process (NZXT CAM) is open and has control of the Kraken Z";
#endif

constexpr quint8 CRITICAL_TEMP = 59;

union CHANNEL_SPEED
{
    quint16  value;
    quint8   data[2];
};

KrakenZDriver::KrakenZDriver(QObject *parent, quint16 VID, quint16 PID)
    : QObject(parent), mFound(false), mInitialized(false), mKrakenDevice(nullptr), mLCDDATA(nullptr), mLCDCTL(nullptr), mLCDIN(nullptr), mLiquidTemp(0), mFanSpeed(0),
      mPumpSpeed(0), mBrightness(50), mRotationOffset(0), mBufferIndex(-1), mImageIndex(1), mBytesLeft(0), mBytesSent(0), mApplyAfterSet(false), mWritingImage(true), mFrames(0), mFPS(0)
{
    QUsb usb;
    auto devices = usb.devices();
    for(const auto& device: qAsConst(devices)){
        if(device.vid == VID && device.pid == PID){
            if(device.configCount > 0){
                auto config = device.configurations[0];
                mKrakenDevice = new QUsbDevice(this);
                QUsb::Id id(PID,VID,device.bus,device.port);
                mKrakenDevice->setId(id);
                config.interface = -1; // claim all interfaces
                mKrakenDevice->setConfig(config);
                mFound = true;
            }
        }
    }
}

void KrakenZDriver::blankScreen()
{
    sendSwitchBucket(0,0);
}


quint16 KrakenZDriver::calculateMemoryStart(quint8 index)
{
    return 800*index; // needs to be 400 - check what the 3401 (query of the assets after they are set)
}

void KrakenZDriver::initialize()
{
    bool errored(true);
    if(mFound && !mInitialized){
        mMeasure.setInterval(1000);
        mMeasure.setSingleShot(false);
        mMeasure.setTimerType(Qt::PreciseTimer);
        connect(&mMeasure, &QTimer::timeout, this, &KrakenZDriver::updateFrameRate);
        if(mKrakenDevice->open() ==  QUsb::statusOK)
        {
            mLCDDATA = new QUsbEndpoint(mKrakenDevice, QUsbEndpoint::bulkEndpoint, 0x02);
            if(!mLCDDATA->open(QIODevice::WriteOnly)){
                qDebug() << "Error opening Bulk write endpoint: " <<  mLCDDATA->errorString();
                qDebug() << mKrakenDevice->id() << mKrakenDevice->config();                                  
            }else { // continue if successfully opened the bulk write
                mLCDCTL = new QUsbEndpoint(mKrakenDevice, QUsbEndpoint::interruptEndpoint, 0x01); // Write
                mLCDIN = new QUsbEndpoint(mKrakenDevice, QUsbEndpoint::interruptEndpoint, 0x81); // Read
                connect(mLCDIN, &QUsbEndpoint::readyRead, this, &KrakenZDriver::receivedControlResponse);
                if(!mLCDCTL->open(QIODevice::WriteOnly) ||  !mLCDIN->open(QIODevice::ReadOnly)){
                    qDebug() << "Failed to open control endpoints: " << mLCDIN->errorString() << " : " << mLCDCTL->errorString();
                    qDebug() << mKrakenDevice->id() << mKrakenDevice->config();
                } else { // continue if sccuessfully opened device
                    sendFWRequest();
                    mInitialized = true;
                    memset(mFrameOut,0,IMAGE_FRAME_SIZE);
                    errored = false;
                }
            }
        }
    }
    if(errored) {
        QJsonObject error_obj;
        if(!mLCDDATA){
            qDebug() << "Failed to open raw usb Kraken device:";
            qDebug() << mKrakenDevice->statusString();
            qDebug() << mKrakenDevice->id() << mKrakenDevice->config();
        }
        error_obj.insert(ID_MESSAGE, OPEN_DENIED_STR);
        emit error(error_obj);
    } else {
        emit deviceReady();
    }

}
void KrakenZDriver::moveToBucket(int bucket)
{
    sendSwitchBucket(bucket);
    mImageIndex = bucket;
}

void KrakenZDriver::setBrightness(quint8 brightness)
{
    if(brightness <= 100 && mBrightness != brightness){
        mBrightness = brightness;
        sendBrightness(brightness);
        emit brightnessChanged(brightness);
    }
}

void KrakenZDriver::setImage(QImage image, quint8 index, bool applyAfterSet)
{
    // Enforce format
    if(image.format() != QImage::Format_RGBA8888){
        image.convertTo(QImage::Format_RGBA8888);
    }
    if(index == mImageIndex){
        index = 0 == index ? 1:0;
    }
    mImageIndex = index;
    emit bucketChanged(mImageIndex);
    sendQueryBucket(mImageIndex);
    mLCDCTL->waitForBytesWritten(1000);
    sendDeleteBucket(mImageIndex);
    mLCDCTL->waitForBytesWritten(1000);
    auto byteCount = image.sizeInBytes();
    sendSetupBucket(mImageIndex, mImageIndex + 1, calculateMemoryStart(mImageIndex) , 400);
    mLCDCTL->waitForBytesWritten(1000);
    sendWriteStartBucket(mImageIndex);
    mLCDCTL->waitForBytesWritten(1000);
    sendBulkDataInfo(2, byteCount);
    mLCDDATA->waitForBytesWritten(1);
    mLCDDATA->writeDataSynchronous(reinterpret_cast<const char*>(image.constBits()), byteCount);
    mLCDDATA->waitForBytesWritten(1);
    sendWriteFinishBucket(index);
    mLCDCTL->waitForBytesWritten(1000);
    ++mFrames;
    if(applyAfterSet) {
        sendSwitchBucket(index);
        mLCDCTL->waitForBytesWritten(1000);
    }
}

void KrakenZDriver::setNZXTMonitor()
{
    setMonitorFPS(false);
    sendSwitchBucket(0,2);
}

void KrakenZDriver::setBuiltIn(quint8 index)
{
    setMonitorFPS(false);
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
        fw[0] = QString::number(data.at(17)).at(0);
        fw[2]=QString::number(data.at(18)).at(0);
        fw[4]= QString::number(data.at(19)).at(0);
        mVersion = fw;
        qDebug() << "KRAKEN FW Version: v" << mVersion;
        emit fwInfoChanged(mVersion);
    }
}

void KrakenZDriver::parseStatus(QByteArray &data)
{
    if(data.size() >= 30)
    {
        //qDebug() << "Status: " << data.toHex();
        qreal temp(float(data[15]) + float(data[16]) / 10);
        if(temp != mLiquidTemp){
            mLiquidTemp = temp;
            emit liquidTemperatureChanged(mLiquidTemp);
        }
        CHANNEL_SPEED speed;
        speed.data[0] = data[17];
        speed.data[1] = data[18];
        if(speed.value != mPumpSpeed){
            mPumpSpeed = speed.value;
            emit pumpSpeedChanged(mPumpSpeed);
        }
        quint8 pump_duty(data[19]);
        if(pump_duty != mPumpDuty){
            mPumpDuty = pump_duty;
            emit pumpDutyChanged(mPumpDuty);
        }
        speed.data[0] = data[23];
        speed.data[1] = data[24];
        if(speed.value != mFanSpeed){
            mFanSpeed = speed.value;
            emit fanSpeedChanged(mFanSpeed);
        }
        auto fan_duty((quint8(data[25])));
        if(fan_duty != mFanDuty){
            mFanDuty = fan_duty;
            emit fanDutyChanged(mFanDuty);
        }
    }
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
    mLCDDATA->writeDataSynchronous(bulk_info, _WRITE_BULK_LENGTH);
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
        QByteArray set_fan;
        set_fan.fill(0x0, _WRITE_LENGTH);
        set_fan[0] = SET_DUTY;
        set_fan[1] = FAN_CHANNEL;
        if(duty != 0){
            memset(&set_fan.data()[4], duty, 44);
        }
        mLCDCTL->write(set_fan);
        mLCDCTL->waitForBytesWritten(1000);
    }
}


void KrakenZDriver::setJsonProfile(QJsonObject profile)
{
    int fanDuty = profile.value("fanDuty").toInt(-1);
    if(fanDuty >= 0 ) {
        setFanDuty(fanDuty);
        mFanDuty = fanDuty;
        emit fanDutyChanged(mFanDuty);
    }
    int pumpDuty = profile.value("pumpDuty").toInt(-1);
    if(pumpDuty) {
        setPumpDuty(pumpDuty);
        mPumpDuty = pumpDuty;
        emit pumpDutyChanged(mFanDuty);
    }
    //set the values
    int rotationOffset = profile.value("rotationOffset").toInt(-1);
    if(rotationOffset >= 0 ) {
        setRotationOffset(rotationOffset);
    }
    int brightness = profile.value("brightness").toInt(-1);
    if(brightness >= 0 ) {
        setBrightness(brightness);
    }
}

void KrakenZDriver::setPumpDuty(quint8 duty)
{
    if(duty >= 20 && duty <= 100){
        QByteArray set_pump;
        set_pump.fill(0x0, _WRITE_LENGTH);
        set_pump[0] = SET_DUTY;
        set_pump[1] = PUMP_CHANNEL;
        memset(&set_pump.data()[4], duty, 44);
        mLCDCTL->write(set_pump);
        mLCDCTL->waitForBytesWritten(1000);
    }
}

void KrakenZDriver::setMonitorFPS(bool monitor)
{
    if(mMeasure.isActive() != monitor) {
        if(monitor) {
            startMonitoringFramerate();
        } else {
            stopMonitoringFramerate();
        }
        emit monitorFPSChanged(monitor);
    }
}

void KrakenZDriver::setScreenOrientation(Qt::ScreenOrientation orientation)
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
    if(mRotationOffset != rotation){
        mRotationOffset = rotation;
        emit rotationOffsetChanged(rotation);
    }
}

void generate_normalized_profile(QList<TempPoint> &normalized, const QList<TempPoint>& profile, quint8 max_first)
{
    if(profile.size() == 0){
        normalized.clear();
        return;
    }
    int c_index = 0;
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
    // final pass makes sure there are no duplicate temps, if a duplicate is found, it will be set to the average of the next two points
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
    request_fw.fill(0x0, _WRITE_LENGTH);
    request_fw[0] = FW_INFO;
    request_fw[1] = FW_REQUEST;
    mLCDCTL->write(request_fw);
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
    mLCDCTL->write(delete_bucket);
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
    mLCDCTL->write(finish_write);
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
    mLCDCTL->write(prepare_write);
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
    mLCDCTL->write(setup_bucket);
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
    mLCDCTL->write(switch_bucket);
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
    mLCDCTL->write(request_status);
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
    mLCDCTL->write(query_bucket);
}


void KrakenZDriver::setRotationOffset(int offset)
{
    if(mRotationOffset != offset) {
        mRotationOffset = offset;
        emit rotationOffsetChanged(offset);
    }
}

QJsonObject KrakenZDriver::toJsonProfile()
{
    QJsonObject out;
    out.insert("pumpDuty", mPumpDuty);
    out.insert("fanDuty", mFanDuty);
    out.insert("rotationOffset", mRotationOffset);
    out.insert("brightness", mBrightness);
    return out;
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
       case RESPONSE_SWITCH_BUCKET:
       case QUERY_RESPONSE:
       case CONFIRM_RESPONSE:
       case SETUP_RESPONSE:
       case RESPONSE_WRITE:
           break;
       default:
       {
           QByteArray cmd;
           cmd.append(data[0]);
           cmd.append(data[1]);

           qDebug() << "Received unknown CMD# " << cmd.toHex();
           qDebug() << "Data " << data.toHex();
           break;
       }
   }
}


KrakenZDriver::~KrakenZDriver()
{
    if(mInitialized){
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
    }
    delete mKrakenDevice;
}
