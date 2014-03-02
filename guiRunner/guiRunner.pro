#-------------------------------------------------
#
# Project created by QtCreator 2013-12-17T14:16:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = guiRunner
TEMPLATE = app
VERSION  = 0.1.0

include(../common.pri)


SOURCES += main.cpp\
        mainwindow.cpp \
        qcustomplot/qcustomplot.cpp

HEADERS  += mainwindow.hpp \
            qcustomplot/qcustomplot.hpp \
            ../settings.h \
            ../functions.h

INCLUDEPATH += qcustomplot

FORMS    += mainwindow.ui

BACKEND_LIB = ../monores/libmonores.a # ../multires/libmultires.a

PRE_TARGETDEPS = $${BACKEND_LIB}
LIBS          += $${BACKEND_LIB}
