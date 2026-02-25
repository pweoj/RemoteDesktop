QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    client.cpp \
    displayui.cpp \
    ffmpeg_server/ffmpeg.cpp \
    loginui.cpp \
    main.cpp \
    mdxgi.cpp \
    server.cpp \
    widget.cpp

HEADERS += \
    client.h \
    displayui.h \
    ffmpeg_server/ffmpeg.h \
    loginui.h \
    mdxgi.h \
    server.h \
    widget.h
LIBS += -ld3d11 -ldxgi
INCLUDEPATH += $$PWD/ffmpeg-8.0.1-full_build-shared/include
LIBS += -L$$PWD/ffmpeg-8.0.1-full_build-shared/lib \
        -lavcodec \
        -lavutil \
        -lswscale

DEFINES += _CRT_SECURE_NO_WARNINGS
CONFIG += link_c++

# 自动复制 DLL
win32 {
    COPIES += FFMpegDlls
    FFMpegDlls.files = $$PWD/ffmpeg-8.0.1-full_build-shared/bin/avcodec-62.dll \
                       $$PWD/ffmpeg-8.0.1-full_build-shared/bin/avutil-60.dll \
                       $$PWD/ffmpeg-8.0.1-full_build-shared/bin/swscale-9.dll
    FFMpegDlls.path = $$OUT_PWD/debug
}
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    displayui.ui \
    loginui.ui
