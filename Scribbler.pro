#-------------------------------------------------
#
# Project created by QtCreator 2015-10-06T12:36:53
#
#-------------------------------------------------

QT       += core gui svg printsupport xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Scribbler
TEMPLATE = app
DEFINES += QT_COMPILING_QSTRING_COMPAT_CPP

SOURCES += main.cpp\
        mainwindow.cpp \
    svgview.cpp \
    preferencesdialog.cpp \
    fontdialog.cpp \
    symboldata.cpp \
    symboldataeditor.cpp

HEADERS  += mainwindow.h \
    svgview.h \
    preferencesdialog.h \
    fontdialog.h \
    symboldata.h \
    symboldataeditor.h

FORMS    += mainwindow.ui \
    preferencesdialog.ui \
    fontdialog.ui

CONFIG   += c++11 static-libstdc++

RC_ICONS = resources\favicon.ico
win32:RC_FILE = Scribbler.rc

RESOURCES += \
    resources/resources.qrc

TRANSLATIONS += Scribbler-ru_RU.ts
