QT += core gui widgets serialport charts bluetooth
QT += 3dcore 3drender 3dinput 3dextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

TARGET = FlightControlPC
TEMPLATE = app

# 源文件
SOURCES += \
    bluetooththread.cpp \
    calibratetab.cpp \
    main.cpp \
    mainwindow.cpp \
    SerialTab.cpp \
    serialthread.cpp \
    FlyCtrlSetTab.cpp \
    FlightStatusTab.cpp \
    AttitudeDialWidget.cpp \
    Drone3DView.cpp \
    AttitudeWaveChart.cpp \
    FlightStatusPanel.cpp \
    ReceiverPanel.cpp \
    SensorDataPanel.cpp \
    GpsInfoPanel.cpp

# 头文件
HEADERS += \
    bluetooththread.h \
    calibratetab.h \
    mainwindow.h \
    SerialTab.h \
    serialthread.h \
    FlyCtrlSetTab.h \
    FlightStatusTab.h \
    AttitudeDialWidget.h \
    Drone3DView.h \
    AttitudeWaveChart.h \
    FlightStatusPanel.h \
    ReceiverPanel.h \
    SensorDataPanel.h \
    GpsInfoPanel.h

# UI文件
FORMS += \
    mainwindow.ui \
    SerialTab.ui \
    flyctrlsettab.ui \
    FlightStatusTab.ui

# 部署配置
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target