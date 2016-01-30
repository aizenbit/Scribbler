#-------------------------------------------------
#
# Project created by QtCreator 2015-10-06T12:36:53
#
#-------------------------------------------------

QT       += core gui svg printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Scribbler
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    svgview.cpp \
    preferencesdialog.cpp \
    fontdialog.cpp \
    letter.cpp \
    svgeditor.cpp

HEADERS  += mainwindow.h \
    svgview.h \
    preferencesdialog.h \
    fontdialog.h \
    letter.h \
    svgeditor.h

FORMS    += mainwindow.ui \
    preferencesdialog.ui \
    fontdialog.ui

CONFIG   += c++11

RC_ICONS = resources\favicon.ico

RESOURCES += \
    resources/resources.qrc

TRANSLATIONS += Scribbler-ru_RU.ts
