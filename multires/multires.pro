TEMPLATE = lib

TARGET   = multires
VERSION  = 0.1.0

include(../common.pri)

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += staticlib

SOURCES += \
    node.cpp \
    multires_grid.cpp

HEADERS += \
    node.hpp \
    multires_grid.hpp
