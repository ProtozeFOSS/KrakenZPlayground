QT += widgets quick multimedia network
CONFIG += c++latest
CONFIG += monitor
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#DEFINES += ENABLE_LOGGING

#DEFINES += _ITERATOR_DEBUG_LEVEL = 1

SOURCES += \
        kraken_appcontroller.cpp \
        krakenz_driver.cpp \
        krakenz_interface.cpp \
        krakenz_software.cpp \
        kzp_controller.cpp \
        main.cpp \
        modules.cpp \
        preview_provider.cpp \
        preview_window.cpp \
        sensor_monitor.cpp \
        settings.cpp \
        system_monitor.cpp \
        system_tray.cpp

RESOURCES += qml.qrc \
    images.qrc

include(QtUsb/src/usb/files.pri)


monitor:linux{
    message("Using Linux Sensor Monitoring")
    INCLUDEPATH += dep/libsensors-cpp/include/
    DEPENDPATH += dep/libsensors-cpp/src/
    SOURCES +=  sensor_reader_linux.cpp \
                dep/libsensors-cpp/src/sensors.cpp \
                dep/libsensors-cpp/src/error.cpp
    LIBS += -lsensors
}

win32:{

    DEFINES +=                                                          \
        _MBCS                                                           \
        WIN32                                                           \
        _CRT_SECURE_NO_WARNINGS                                         \
        _WINSOCK_DEPRECATED_NO_WARNINGS                                 \
        WIN32_LEAN_AND_MEAN                                             \
    LIBS +=                                                             \
        -lws2_32                                                        \
        -lole32
}

monitor:win32{
    message("Using LHWM sensor monitoring (Windows)")
    SOURCES += \
    sensor_reader_windows.cpp
    INCLUDEPATH += lhwm-cpp-wrapper
    DEPENDPATH += lhwm-cpp-wrapper
    HEADERS += lhwm-cpp-wrapper/lhwm-cpp-wrapper.h
    INCLUDEPATH += "C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8/Include/um"
    LIBS += "C:/Program Files (x86)/Windows Kits/NETFXSDK/4.8/Lib/um/x64/mscoree.lib"
    CONFIG += embed_manifest_exe
    QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'
}

monitor:win32:CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/lhwm-cpp-wrapper/x64/Debug -llhwm-cpp-wrapper
}

monitor:win32:CONFIG(release, debug|release) {
    LIBS += -L$$PWD/lhwm-cpp-wrapper/x64/Release -llhwm-cpp-wrapper
}


# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    kraken_appcontroller.h \
    krakenz_driver.h \
    krakenz_interface.h \
    krakenz_software.h \
    kzp_controller.h \
    kzp_coroutines.h \
    kzp_keys.h \
    modules.h \
    preview_provider.h \
    preview_window.h \
    sensor_controller.h \
    sensor_monitor.h \
    settings.h \
    system_monitor.h \
    system_tray.h

