TEMPLATE = lib

TARGET   = monores
VERSION  = 0.1.0

include(../common.pri)

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += staticlib

SOURCES += \
    monores_grid.cpp

HEADERS += \
    monores_grid.hpp
