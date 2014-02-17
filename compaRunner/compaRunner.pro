TEMPLATE = app

TARGET   = compaRunner
VERSION  = 0.1.0

include(../common.pri)

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

HEADERS += ../settings.h \
           ../functions.h

BACKEND_LIB  = ../node/libnode.a
BACKEND_LIB += ../regular/libregular.a

PRE_TARGETDEPS = $${BACKEND_LIB}
LIBS          += $${BACKEND_LIB}

OTHER_FILES += \
    plots.gp
