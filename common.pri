QMAKE_LFLAGS   += -std=c++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wold-style-cast -Wall -Wuninitialized -Wextra
QMAKE_CXXFLAGS_RELEASE += -funroll-loops # unroll short, iterative for-loops
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS_DEBUG   += -O0

macx {
  CONFIG += c++11
  INCLUDEPATH += /usr/local/Cellar/boost/1.55.0_1/include/ # brew install boost
}

contains( QMAKE_CC, gcc ) {
    # http://gcc.gnu.org/bugzilla/show_bug.cgi?id=55805
    QMAKE_CXXFLAGS += -Wno-missing-field-initializers
}

# enable openmp
QMAKE_CXXFLAGS += -fopenmp
LIBS += -lgomp
# LIBS += -L/usr/local/lib64 # -liomp5

# to get openmp working with clang:
# - compile llvm/clang from: http://clang-omp.github.io/#try-openmp-clang
# to use the intel omp library instead of gomp with is provided by gcc
# - compile libiomp5 (intels omp library) from: https://www.openmprtl.org/download
#   (make sure that iomp5.so is somewhere in /usr/local/lib(64) and omp.h is in /usr/local/include)

INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD

HEADERS += \
    $$PWD/settings.h \
    $$PWD/functions.h \
    $$PWD/theory.hpp \
    $$PWD/point.hpp \
    $$PWD/grid.hpp

Release:DEFINES += NDEBUG

SOURCES += \
    $$PWD/grid.cpp
