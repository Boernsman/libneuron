// MIT License
//
// Copyright (c) 2023 Bernhard Trinnes
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef NEURONSPI_H
#define NEURONSPI_H

#include <QObject>
#include <QLoggingCategory>
#include <QSocketNotifier>
#include <QFile>
#include <QDir>

#include "spi.h"

Q_DECLARE_LOGGING_CATEGORY(dcNeuronSpi)

class NeuronInterrupt;

class NeuronSpi : public QObject
{
    Q_OBJECT
public:
    explicit NeuronSpi(int index, QObject *parent = nullptr);

    bool init();

    SpiMessage *writeBit(quint16 reg, quint8 value);
    SpiMessage *readRegisters(uint16_t reg, uint8_t cnt);
    //bool readRegisters(uint16_t reg, uint8_t cnt, uint16_t* result);
    bool writeRegister(uint16_t reg, uint16_t value);
    bool writeRegisters(uint16_t reg, uint8_t cnt, uint16_t* values);

    bool readBits(uint16_t reg, uint16_t cnt, uint8_t* result);
   // bool writeBit(uint16_t reg, uint8_t value);
    bool writeBits(uint16_t reg, uint16_t cnt, uint8_t* values);

    bool idleOperation();

private:
    typedef struct {
        uint8_t op;
        uint8_t len;
        uint16_t reg;
    } __attribute__((packed)) CommunicationHeader;
    const int m_sizeOfCommunicationHeader = 4;

    typedef struct {
        uint8_t op;
        uint8_t len;
        uint16_t reg;
        uint16_t crc;
    } __attribute__((packed)) NeuronSpiMessage;
    const int m_sizeOfNeuronSpiMessage = 6;

    Spi *m_spi = nullptr;
    const int m_index;
    const int m_defaultSpiSpeed = 8000000; //8 MHz


    NeuronInterrupt *m_neuronInterrupt =  nullptr;

    int m_gpio;
    NeuronSpiMessage m_tx1;
    NeuronSpiMessage m_rx1;

    uint8_t m_tx2[256 + 2 + 40];
    uint8_t m_rx2[256 + 2 + 40];
};

class NeuronInterrupt: public QObject
{
    Q_OBJECT
public:
    explicit NeuronInterrupt(int gpio, QObject *parent = nullptr);
    bool init();

private:
    int m_gpio;
    QDir m_gpioDirectory;
    QFile m_gpioValueFile;
    QSocketNotifier *m_notifier;

signals:
    void interruptReceived();

private slots:
    void onSocketNotifierActivated(const int &value);
};

#endif // NEURONSPI_H
