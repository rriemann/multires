TEMPLATE = lib

TARGET   = node
VERSION  = 0.1.0

include(../common.pri)

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += staticlib

SOURCES += \
    node_base.cpp

HEADERS += \
    node_base.hpp \
    node_iterator.hpp
