#-------------------------------------------------
#
# Project created by QtCreator 2013-06-20T17:42:25
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = lpcprog
TEMPLATE = app


SOURCES += main.cpp qappmainwindow.cpp qlpcprog.cpp qhexloader.cpp
HEADERS +=            qappmainwindow.h qlpcprog.h   qhexloader.h
FORMS   +=            qappmainwindow.ui
