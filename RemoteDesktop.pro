QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    client.cpp \
    displayui.cpp \
    loginui.cpp \
    main.cpp \
    mdxgi.cpp \
    server.cpp \
    widget.cpp

HEADERS += \
    client.h \
    displayui.h \
    loginui.h \
    mdxgi.h \
    server.h \
    widget.h
LIBS += -ld3d11 -ldxgi
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    displayui.ui \
    loginui.ui
