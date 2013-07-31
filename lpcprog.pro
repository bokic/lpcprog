#-------------------------------------------------
#
# Project created by QtCreator 2013-06-20T17:42:25
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = lpcprog
TEMPLATE = app


SOURCES += main.cpp qappmainwindow.cpp qlpcprog.cpp qhexloader.cpp
HEADERS +=          qappmainwindow.h   qlpcprog.h   qhexloader.h
FORMS   +=          qappmainwindow.ui

equals(QT_MAJOR_VERSION, 4) {
    unix:!symbian {
        HEADERS += \
            qt4/qt-qtserialport/src/serialport/qttylocker_unix_p.h \
            qt4/qt-qtserialport/src/serialport/qserialport.h \
            qt4/qt-qtserialport/src/serialport/qserialport_p.h \
            qt4/qt-qtserialport/src/serialport/qserialportinfo.h \
            qt4/qt-qtserialport/src/serialport/qserialportinfo_p.h

        SOURCES += \
            qt4/qt-qtserialport/src/serialport/qttylocker_unix.cpp \
            qt4/qt-qtserialport/src/serialport/qserialport.cpp \
            qt4/qt-qtserialport/src/serialport/qserialport_unix.cpp \
            qt4/qt-qtserialport/src/serialport/qserialportinfo.cpp \
            qt4/qt-qtserialport/src/serialport/qserialportinfo_unix.cpp

        INCLUDEPATH += \
            qt4/qt-qtserialport/src/serialport/qt4support \
            qt4/qt-qtserialport/src/serialport
    } else {
        error("Qt4 is supported only under Linux OS.")
    }
}
