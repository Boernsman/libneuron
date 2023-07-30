include(../libneuron.pri)

TARGET = libneuron-tests

QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

INCLUDEPATH += $$top_srcdir/libneuron/
LIBS += -L$$top_builddir/libneuron/ -lneuron

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        configuration.cpp \
        main.cpp \
        modbusmap.cpp \
        testengine.cpp

HEADERS += \
    configuration.h \
    modbusmap.h \
    testengine.h

target.path = $$[QT_INSTALL_PREFIX]/bin
INSTALLS += target

DISTFILES += \
    test.json
