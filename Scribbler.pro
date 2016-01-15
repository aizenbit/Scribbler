#-------------------------------------------------
#
# Project created by QtCreator 2015-10-06T12:36:53
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Scribbler
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    manuscript.cpp \
    svgview.cpp \
    preferencesdialog.cpp

HEADERS  += mainwindow.h \
    manuscript.h \
    svgview.h \
    preferencesdialog.h

FORMS    += mainwindow.ui \
    preferencesdialog.ui

CONFIG   += c++11

RC_ICONS = resources\favicon.ico
