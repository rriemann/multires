TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
#    node-bak.cpp \
    node_base.cpp

HEADERS += \
    settings.h \
#    node-bak.h \
    node_base.hpp \
    node_iterator.hpp


QMAKE_LFLAGS   += -std=c++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wold-style-cast
