QT       += core gui network websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    command.cpp \
    main.cpp \
    mainwindow.cpp \
    multi_combo_box.cpp \
    mythread.cpp \
    tools.cpp \
    video_list_dialog.cpp \
    ws_client.cpp \
    event_thread.cpp

HEADERS += \
    mainwindow.h \
    multi_combo_box.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    SophonOS_zh_CN.ts

QMAKE_CXXFLAGS += -g

DEFINES += ARM64

contains(DEFINES, ARM64) {
BMNNSDK_PATH=/bmnnsdk2/
QMAKE_RPATH += $${BMNNSDK_PATH}/lib/bmnn/soc
QMAKE_LFLAGS += -Wl,-rpath=$${BMNNSDK_PATH}/lib/bmnn/soc
QMAKE_LFLAGS += -Wl,-rpath=$${BMNNSDK_PATH}/lib/decode/soc
QMAKE_LFLAGS += -L$${BMNNSDK_PATH}/lib/bmnn/soc -lbmcpu  -lbmcv  -lbmlib  -lbmrt  -lusercpu
QMAKE_LFLAGS += -L$${BMNNSDK_PATH}/lib/decode/soc \
        -lbmion -lbmjpulite -lbmvpuapi -lvideo_bm -lyuv \
        -lbmjpuapi  -lbmvideo -lbmvpulite
LIBS += $${BMNNSDK_PATH}/lib/opencv/soc/*.so \
        $${BMNNSDK_PATH}/lib/ffmpeg/soc/*.so
INCLUDEPATH += $${BMNNSDK_PATH}/include/opencv/opencv4 \
        $${BMNNSDK_PATH}/include/ffmpeg \
        $${BMNNSDK_PATH}/include/bmlib
}

# Default rules for deployment.

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icon.qrc
