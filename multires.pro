TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
#    node-bak.cpp \
    node_base.cpp \
    node_real.cpp

HEADERS += \
    settings.h \
#    node-bak.h \
    node_base.hpp \
    node_real.hpp


QMAKE_LFLAGS   += -std=c++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wold-style-cast -Wall -Wuninitialized -Wextra
QMAKE_CXXFLAGS_RELEASE += -funroll-loops # unroll short, iterative for-loops
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
