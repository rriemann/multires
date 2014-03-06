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

BACKEND_LIB  = ../multires/libmultires.a
BACKEND_LIB += ../monores/libmonores.a

PRE_TARGETDEPS = $${BACKEND_LIB}
LIBS          += $${BACKEND_LIB}

OTHER_FILES += \
    plots.gp \
    colormap.gp

#QMAKE_CLEAN += \
#    output.dat
