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
QMAKE_CXXFLAGS += -Wold-style-cast -Wall -Wuninitialized -Wextra
QMAKE_CXXFLAGS_RELEASE += -funroll-loops # unroll short, iterative for-loops
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

contains( QMAKE_CC, gcc ) {
    # http://gcc.gnu.org/bugzilla/show_bug.cgi?id=55805
    QMAKE_CXXFLAGS += -Wno-missing-field-initializers
}
