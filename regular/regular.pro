TEMPLATE = lib

TARGET   = regular
VERSION  = 0.1.0

include(../common.pri)

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += staticlib

SOURCES += \
    regular_base.cpp

HEADERS += \
    regular_base.hpp
