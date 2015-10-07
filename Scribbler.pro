#-------------------------------------------------
#
# Project created by QtCreator 2015-10-06T12:36:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Scribbler
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    manuscript.cpp \
    graphics_view_zoom.cpp

HEADERS  += mainwindow.h \
    manuscript.h \
    graphics_view_zoom.h

FORMS    += mainwindow.ui

CONFIG   += c++11
