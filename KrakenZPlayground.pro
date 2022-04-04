QT += widgets quick multimedia webengine

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#DEFINES += ENABLE_LOGGING

SOURCES += \
        offscreen_appcontroller.cpp \
        krakenimageprovider.cpp \
        krakenzdriver.cpp \
        main.cpp \
        systemtray.cpp

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
    offscreen_appcontroller.h \
    krakenimageprovider.h \
    krakenzdriver.h \
    systemtray.h

DISTFILES += \
    examples/Clock/Clock.qml \
    examples/KZP_Clock/kzp_clock.qml \
    examples/Krakify/Krakify.qml \
    examples/LCARS_Clock/LCARS_Clock.qml \
    examples/Monitor/Monitor.qml \
