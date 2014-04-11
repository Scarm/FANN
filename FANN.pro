#-------------------------------------------------
#
# Project created by QtCreator 2014-01-21T12:21:42
#
#-------------------------------------------------
QT       += core

TARGET = FANN
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIB += -fopenmp
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

SOURCES += main.cpp

HEADERS += \
    ImageTransform.hpp \
    Working/Layer.hpp \
    Working/Activator.hpp \
    Working/Connector.hpp \
    Working/BaseNetwork.hpp \
    Working/Network.hpp


















