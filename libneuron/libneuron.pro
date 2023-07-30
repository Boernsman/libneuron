TARGET = neuron
TEMPLATE = lib
CONFIG += staticlib

QT += core serialport

QMAKE_CXXFLAGS *= -Werror -std=c++11 -g
QMAKE_LFLAGS *= -std=c++11

DEFINES += VERSION_STRING=\\\"$${VERSION_STRING}\\\"

HEADERS += \
    neurondefines.h \
    neuronspi.h \
    neuronutil.h \
    spi.h \
    spimessage.h

SOURCES += \
    neuronspi.cpp \
    neuronutil.cpp \
    spi.cpp \
    spimessage.cpp

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

for(header, HEADERS) {
    path = $$[QT_INSTALL_PREFIX]/include/libneuron/$${dirname(header)}
    eval(headers_$${path}.files += $${header})
    eval(headers_$${path}.path = $${path})
    eval(INSTALLS *= headers_$${path})
}
