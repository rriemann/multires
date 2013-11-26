TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    property.cpp \
    node.cpp \
    nodeiterator.cpp

HEADERS += \
    settings.h \
    property.h \
    node.h \
    nodeiterator.h


QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS   += -std=c++11
