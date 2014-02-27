TEMPLATE = app

TARGET   = rawRunner
VERSION  = 0.1.0

include(../common.pri)

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

HEADERS += ../settings.h \
           ../functions.h

contains(DEFINES, REGULAR) {
    BACKEND_LIB = ../monores/libmonores.a
} else {
    BACKEND_LIB = ../multires/libmultires.a
}

PRE_TARGETDEPS = $${BACKEND_LIB}
LIBS          += $${BACKEND_LIB}
