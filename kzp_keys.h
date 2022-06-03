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
    static QString ContentType{QStringLiteral("contentType")};
    static QString Type{QStringLiteral("type")};
    static QString FilePath{QStringLiteral("filePath")};
    static QString Path{QStringLiteral("path")};
    static QString Profiles{QStringLiteral("profiles")};
    static QString ErrorString{QStringLiteral("errorString")};
    static QString Message{QStringLiteral("message")};
    static QString Size{QStringLiteral("size")};
    static QString Name{QStringLiteral("name")};
    static QString Device{QStringLiteral("device")};
    static QString Folder{QStringLiteral("folder")};
    static QString Version{QStringLiteral("version")};
    static QString VersionText{QStringLiteral("versionText")};
    static QString Settings{QStringLiteral("settings")};
    static QString Entry{QStringLiteral("entry")};
    static QString Url{QStringLiteral("url")};
    static QString Icon{QStringLiteral("icon")};
    static QString Files{QStringLiteral("files")};
    static QString Dependencies{QStringLiteral("dependencies")};
    static QString X{QStringLiteral("x")};
    static QString Y{QStringLiteral("y")};

    // AIO Related
    static QString PumpDuty{QStringLiteral("pumpDuty")};
    static QString FanDuty{QStringLiteral("fanDuty")};
    static QString Rotation{QStringLiteral("rotationOffset")};
    static QString Brightness{QStringLiteral("brightness")};
    static QString Software{QStringLiteral("software")};

};

#endif // KZP_KEYS_H
