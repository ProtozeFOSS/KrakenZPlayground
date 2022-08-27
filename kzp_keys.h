#ifndef KZP_KEYS_H
#define KZP_KEYS_H
// Shared constant character pointers (optimization technique) meant to facilitate global string keys
// using QStringLiteral to be inserted into the data header segment
// increase binary size, reduce creation overhead while adding support for auto complete on string keys
// Having a defined Name allows for compile time checks on the value (used by the developer) and reduces chance
// of using an incorrect label i.e. "Type" vs "type" or "errorstring" vs "errorString"
// Especially helpful for working with JSON properties
// Downside, compiler has issues tracking use from global state. The occasional manual audit is useful for "global keys"
// search "SharedKeys::Term" as a term to check usage
#include <QString>

// Values use Camel Case format (ObjectName , "stringValue")
// (Future) Could be generated
namespace SharedKeys{
// General
    static QString Attributes{QStringLiteral("attributes")};
    static QString ContentType{QStringLiteral("contentType")};
    static QString Type{QStringLiteral("type")};
    static QString SubType{QStringLiteral("subType")};
    static QString FilePath{QStringLiteral("filePath")};
    static QString Path{QStringLiteral("path")};
    static QString Profiles{QStringLiteral("profiles")};
    static QString ErrorString{QStringLiteral("errorString")};
    static QString Message{QStringLiteral("message")};
    static QString Size{QStringLiteral("size")};
    static QString Name{QStringLiteral("name")};
    static QString Sensor{QStringLiteral("sensor")};
    static QString Sensors{QStringLiteral("sensors")};
    static QString Value{QStringLiteral("value")};
    static QString Label{QStringLiteral("label")};
    static QString ReadOnly{QStringLiteral("readOnly")};
    static QString Device{QStringLiteral("device")};
    static QString Folder{QStringLiteral("folder")};
    static QString Version{QStringLiteral("version")};
    static QString VersionText{QStringLiteral("versionText")};
    static QString Settings{QStringLiteral("settings")};
    static QString Entry{QStringLiteral("entry")};
    static QString Url{QStringLiteral("url")};
    static QString Icon{QStringLiteral("icon")};
    static QString Files{QStringLiteral("files")};
    static QString Images{QStringLiteral("images")};
    static QString Modules{QStringLiteral("modules")};
    static QString Dependencies{QStringLiteral("dependencies")};
    static QString X{QStringLiteral("x")};
    static QString Y{QStringLiteral("y")};
    static QString PollDelay{QStringLiteral("pollDelay")};

    // AIO Related
    static QString PumpDuty{QStringLiteral("pumpDuty")};
    static QString FanDuty{QStringLiteral("fanDuty")};
    static QString Rotation{QStringLiteral("rotationOffset")};
    static QString Brightness{QStringLiteral("brightness")};
    static QString Software{QStringLiteral("software")};
    static QString LMSensors{QStringLiteral("linux-sensors")};
    static QString LHWMSensors{QStringLiteral("windows-sensors")};

    // Sensor Types
    static QString CPUDieTemp{QStringLiteral("CPU.DieTemp")};
    static QString CPUPackageTemp{QStringLiteral("CPU.PackageTemp")};
    static QString CPUAverageTemp{QStringLiteral("CPU.AverageTemp")};
    static QString CPUWorkLoad{QStringLiteral("CPU.WorkLoad")};
    static QString CPUWorkLoadTotal{QStringLiteral("CPU.WorkLoadTotal")};
    static QString CPUFrequency{QStringLiteral("CPU.Frequency")};
    static QString GPUCoreTemp{QStringLiteral("GPU.CoreTemp")};
    static QString GPUJunctionTemp{QStringLiteral("GPU.JunctionTemp")};
    static QString GPUMemoryTemp{QStringLiteral("GPU.MemoryTemp")};
    static QString GPUMemorySpeed{QStringLiteral("GPU.MemorySpeed")};
    static QString GPUFanSpeed{QStringLiteral("GPU.FanSpeed")};
    static QString GPUClockSpeed{QStringLiteral("GPU.ClockSpeed")};
    static QString GPUMemoryUsed{QStringLiteral("GPU.MemoryUsed")};
    static QString GPUWorkLoad{QStringLiteral("GPU.WorkLoad")};
    static QString GPUFrequency{QStringLiteral("GPU.Frequency")};
    static QString GPUVoltage{QStringLiteral("GPU.Voltage")};
    static QString GPUWattage{QStringLiteral("GPU.Wattage")};

    static QString Temperature{QStringLiteral("Temperature")};
    static QString Utilization{QStringLiteral("Utilization")};
    static QString Speed{QStringLiteral("Speed")};
    static QString DataSize{QStringLiteral("Data Size")};
    static QString Frequency{QStringLiteral("Frequency")};
    static QString Voltage{QStringLiteral("Voltage")};
    static QString Wattage{QStringLiteral("Wattage")};
    static QString Current{QStringLiteral("Current")};
    static QString Minimum{QStringLiteral("Minimum")};
    static QString Maximum{QStringLiteral("Maximum")};
    static QString Highest{QStringLiteral("Highest")};
    static QString Lowest{QStringLiteral("Lowest")};
    static QString TypeID{QStringLiteral("TypeID")};
    static QString Data{QStringLiteral("Data")};
    static QString DataRate{QStringLiteral("DataRate")};

#ifdef Q_OS_LINUX
    static QString SensorSettings{QStringLiteral("hmonSensors")};
#endif
#ifdef Q_OS_WINDOWS
    static QString SensorSettings{QStringLiteral("lhwmSensors")};
#endif

};

#endif // KZP_KEYS_H
