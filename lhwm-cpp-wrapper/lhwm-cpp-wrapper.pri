win32:contains(QMAKE_TARGET.arch, x86_64) {
    LIBS +=                                                             \
        -lws2_32                                                        \
        -lole32                                                         \
}

win32:contains(QMAKE_TARGET.arch, x86) {
    LIBS +=                                                             \
        -lws2_32                                                        \
        -lole32                                                         \
}

win32:DEFINES +=                                                        \
    _MBCS                                                               \
    WIN32                                                               \
    _CRT_SECURE_NO_WARNINGS                                             \
    _WINSOCK_DEPRECATED_NO_WARNINGS                                     \
    WIN32_LEAN_AND_MEAN                                                 \

win32:{
    INCLUDEPATH += $$PWD
    DEPENDPATH +=  $$PWD
    HEADERS += $$PWD/lhwm-cpp-wrapper.h
}

win32:CONFIG(debug, debug|release):contains(QMAKE_TARGET.arch, x86_64) {
    LIBS += -L$$PWD/x64/Debug -llhwm-cpp-wrapper
}

win32:CONFIG(release, debug|release):contains(QMAKE_TARGET.arch, x86_64) {
    LIBS += -L$$PWD/x64/Release -llhwm-cpp-wrapper
}
