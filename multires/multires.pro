TEMPLATE = lib

TARGET   = multires
VERSION  = 0.1.0

include(../common.pri)

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += staticlib

SOURCES += \
    multires.cpp

HEADERS += \
    multires.hpp \
    node_iterator.hpp
