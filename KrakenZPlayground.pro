QT += widgets quick multimedia network

CONFIG += c++20

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#DEFINES += ENABLE_LOGGING

SOURCES += \
        kraken_appcontroller.cpp \
        krakenz_driver.cpp \
        krakenz_interface.cpp \
        krakenz_software.cpp \
        kzp_controller.cpp \
        main.cpp \
        modules.cpp \
        preview_provider.cpp \
        settings.cpp \
        system_tray.cpp

RESOURCES += qml.qrc \
    images.qrc

include(QtUsb/src/usb/files.pri)

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
    settings.h \
    system_tray.h

DISTFILES += \
    examples/Clock/Clock.qml \
    examples/KZP_Clock/kzp_clock.qml \
    examples/Krakify/Krakify.qml \
    examples/LCARS_Clock/LCARS_Clock.qml \
    examples/Monitor/Monitor.qml \
