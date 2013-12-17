TEMPLATE = app

TARGET   = rawRunner
VERSION  = 0.1.0

include(../common.pri)

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

NODE_LIB = ../node/libnode.a

PRE_TARGETDEPS = $${NODE_LIB}
LIBS          += $${NODE_LIB}
