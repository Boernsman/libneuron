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

#include "neuronspi.h"
#include "neuronutil.h"

#include <QDebug>

Q_LOGGING_CATEGORY(dcNeuronSpi, "NeuronSpi")

NeuronSpi::NeuronSpi(int index, QObject *parent) :
    QObject{parent},
    m_index(index)
{
    switch (index) {
    case 0:
        m_spi = new Spi("/dev/spidev0.1", this);
        m_gpio = 27;
        break;
    case 1:
        m_spi = new Spi("/dev/spidev0.3", this);
        m_gpio = 23;
        break;
    case 2:
        m_spi = new Spi("/dev/spidev0.2", this);
        m_gpio = 22;
        break;
    default:
        m_spi = new Spi("", this);
        qCWarning(dcNeuronSpi()) << "Index out of range [0, 2]";
    }
}

bool NeuronSpi::init()
{
    if (!m_spi->init()) {
        return false;
    }
    m_spi->start();

    // Read firmware and hardware versions
    SpiMessage * message = readRegisters(1000, 5);
    connect(message, &SpiMessage::finished, this, [=] {
        uint16_t *configRegisters = (uint16_t *)message->rxData();
        auto boardVersion = NeuronUtil::parseVersion(configRegisters);
        int speed = NeuronUtil::getBoardSpeed(boardVersion);
        qCInfo(dcNeuronSpi()) << "Digital Inputs:" << boardVersion.DiCount;
        qCInfo(dcNeuronSpi()) << "Digital Outputs:" << boardVersion.DoCount;
        qCInfo(dcNeuronSpi()) << "Analog Inputs:" << boardVersion.AiCount;
        qCInfo(dcNeuronSpi()) << "Analog Outputs:" << boardVersion.AoCount;
        qCInfo(dcNeuronSpi()) << "UART interfaces:" << boardVersion.UartCount;
        qCInfo(dcNeuronSpi()) << "Firmware:" << QString("%1.%2").arg(SW_MAJOR(boardVersion.SwVersion)).arg(SW_MINOR(boardVersion.SwVersion));
        qCInfo(dcNeuronSpi()) << "Hardware:" << QString("%1.%2").arg(HW_BOARD(boardVersion.HwVersion)).arg(HW_MAJOR(boardVersion.HwVersion));
        qCInfo(dcNeuronSpi()) << "SPI speed:" << speed/1000000 << "MHz";
        qCInfo(dcNeuronSpi()) << "Hardware supports increased SPI speed of" << speed/1000000 <<"MHz";

        if (!m_spi->setSpiSpeed(speed)) {
            qCWarning(dcNeuronSpi()) << "Could not set SPI speed.";
        }

        /* TODO: Check if the registers can still be read
         *    if (!readRegisters(1000, 5, configRegisters)) {
        qCWarning(dcNeuronSpi()) << "Could not read register 1000";
        return false;
        {
        qCWarning(dcNeuronSpi()) << "Could not read register 1000 with new SPI speed, setting back to default speed:" << m_defaultSpiSpeed;
        m_spi->setSpiSpeed(m_defaultSpiSpeed);
        speed = m_defaultSpiSpeed;
        */
        delete message;
     });


    // The interrupt is not yet used, this code is kept as a reminder to improve the code further.
    m_neuronInterrupt = new NeuronInterrupt(m_gpio, this);
    if (!m_neuronInterrupt->init()) {
        qCWarning(dcNeuronSpi()) << "Could not init NeuronInterrupt";
        return false;
    }
    connect(m_neuronInterrupt, &NeuronInterrupt::interruptReceived, this, [=] {
        qCDebug(dcNeuronSpi()) << "Interrupt received, not yet implemented";
    });

    return true;
}

SpiMessage *NeuronSpi::readRegisters(uint16_t reg, uint8_t cnt)
{
    //uint16_t len2 = m_sizeOfCommunicationHeader + sizeof(uint16_t) * cnt;
    //if (!twoPhaseOperation(FunctionCode::ReadRegister, reg, len2))
    //    return false;
    SpiMessage *message = new SpiMessage(FunctionCode::ReadRegister, reg, cnt, this);
    m_spi->sendMessage(message);
    /*if ((((CommunicationHeader *)(m_rx2))->op != FunctionCode::ReadRegister) ||
            (((CommunicationHeader *)(m_rx2))->len > cnt) ||
            (((CommunicationHeader *)(m_rx2))->reg != reg)) {
        qCWarning(dcNeuronSpi()) << "Unexpected reply when reading register";
        return false;
    }
    cnt = ((CommunicationHeader *)(m_rx2))->len;
    memmove(result, m_rx2+m_sizeOfCommunicationHeader, cnt * sizeof(uint16_t));
    */
    return message;
}

bool NeuronSpi::writeRegister(uint16_t reg, uint16_t value)
{
    Q_UNUSED(reg)
    Q_UNUSED(value)
    return true; //onePhaseOperation(FunctionCode::WriteRegister, reg, value);
}

bool NeuronSpi::writeRegisters(uint16_t reg, uint8_t cnt, uint16_t *values)
{
    if (cnt > 126) {
        qCWarning(dcNeuronSpi()) << "Too many registers in WRITE_REG";
        return false;
    }
    Q_UNUSED(reg)
    Q_UNUSED(values)
    //uint16_t len2 = m_sizeOfCommunicationHeader + sizeof(uint16_t) * cnt;

    ((CommunicationHeader *)(m_rx2))->len = cnt;
    memmove(m_tx2 + m_sizeOfCommunicationHeader, values, cnt * sizeof(uint16_t));

    //if (!twoPhaseOperation(FunctionCode::WriteRegister, reg, len2))
    //    return false;

    if (((CommunicationHeader *)(m_rx2))->op != FunctionCode::WriteRegister) {
        qCWarning(dcNeuronSpi()) << "Unexpected reply in WRITE_REG";
        return false;
    }
    return true;
}


bool NeuronSpi::readBits(uint16_t reg, uint16_t cnt, uint8_t *result)
{
    uint16_t len2 = m_sizeOfCommunicationHeader + (((cnt+15) >> 4) << 1);  // trunc to 16bit in bytes
    if (len2 > 256) {
        qCWarning(dcNeuronSpi()) << "Too many registers in READ_BITS";
        return false;
    }
    //if (!twoPhaseOperation(FunctionCode::ReadBit, reg, len2))
    //        return false;

    if ((((CommunicationHeader *)(m_rx2))->op != FunctionCode::ReadBit) ||
            (((CommunicationHeader *)(m_rx2))->reg != reg)) {
        qCWarning(dcNeuronSpi()) << "Unexpected reply in READ_BIT";
        return false;
    }
    cnt = ((CommunicationHeader *)(m_rx2))->len;
    memmove(result, m_rx2+m_sizeOfCommunicationHeader, ((cnt+7) >> 3));    // trunc to 8 bit

    return true;
}

SpiMessage* NeuronSpi::writeBit(quint16 reg, quint8 value)
{
    SpiMessage *message = new SpiMessage(FunctionCode::WriteBit, reg, value, this);
    m_spi->sendMessage(message);
    return message;//onePhaseOperation(FunctionCode::WriteBit, reg, value);
}

bool NeuronSpi::writeBits(uint16_t reg, uint16_t cnt, uint8_t *values)
{
    Q_UNUSED(reg)
    uint16_t len2 = m_sizeOfCommunicationHeader + (((cnt+15) >> 4) << 1);  // trunc to 16bit in bytes
    if (len2 > 256) {
        qCWarning(dcNeuronSpi()) << "Too many registers in WRITE_BITS";
        return false;
    }

    ((CommunicationHeader *)(m_rx2))->len = cnt;
    memmove(m_tx2 + m_sizeOfCommunicationHeader, values, ((cnt+7) >> 3));

    //if (!twoPhaseOperation(FunctionCode::WriteBits, reg, len2))
    //    return false;

    if (((CommunicationHeader *)(m_rx2))->op != FunctionCode::WriteBits) {
        qCWarning(dcNeuronSpi()) << "Unexpected reply in WRITE_BITS";
        return false;
    }
    return true;
}

bool NeuronSpi::idleOperation()
{
    //onePhaseOperation(FunctionCode::Idle, 0x0e55, 0);
    return true;
}

NeuronInterrupt::NeuronInterrupt(int gpio, QObject *parent) :
    QObject{parent},
    m_gpio(gpio),
    m_gpioDirectory(QDir(QString("/sys/class/gpio/gpio%1").arg(QString::number(gpio))))
{
    m_gpioValueFile.setFileName(QString("/sys/class/gpio/gpio%1/value").arg(QString::number(gpio)));
}


bool NeuronInterrupt::init()
{
    qCInfo(dcNeuronSpi()) << "Initializing Neuron interrupt pin" << m_gpioValueFile.fileName();

    QFile exportFile("/sys/class/gpio/export");
    if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(dcNeuronSpi()) << "Could not open GPIO export file:" << exportFile.errorString();
        return false;
    }

    QTextStream exportOut(&exportFile);
    exportOut << m_gpio;
    exportFile.close();

    if (!m_gpioValueFile.exists()) {
        qCWarning(dcNeuronSpi()) << "Neuron interrupt gpio does not exist" << m_gpioValueFile.fileName();
        return false;
    }

    QFile directionFile(m_gpioDirectory.path() + QDir::separator() + "direction");
    if (!directionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(dcNeuronSpi()) << "Could not open Neuron interrupt direction file:" << directionFile.errorString();
        return false;
    }

    QTextStream directionOut(&directionFile);
    directionOut << "in";
    directionFile.close();


    if (!m_gpioValueFile.open(QFile::ReadOnly)) {
        qCWarning(dcNeuronSpi()) << "Could not open Neuron interrupt gpio:" << m_gpioValueFile.fileName();
        return false;
    }

    m_notifier = new QSocketNotifier(m_gpioValueFile.handle(), QSocketNotifier::Exception);
    connect(m_notifier, &QSocketNotifier::activated, this, &NeuronInterrupt::onSocketNotifierActivated);

    qCDebug(dcNeuronSpi()) << "Socket notififier started";
    m_notifier->setEnabled(true);
    return true;
}

void NeuronInterrupt::onSocketNotifierActivated(const int &value)
{
    Q_UNUSED(value)
    m_gpioValueFile.seek(0);
    QByteArray data = m_gpioValueFile.readAll();

    if (data[0] == '1') {
        emit interruptReceived();
    }
}
