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

#include "spi.h"

extern "C" {
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
}

Q_LOGGING_CATEGORY(dcSpi, "Spi")

Spi::Spi(const QString &spiDevicePath, QObject *parent)
    : QThread{parent}
{
    m_spiDevice.setFileName(spiDevicePath);
}

bool Spi::init()
{
    qCInfo(dcSpi()) << "Initializing SPI interface" << m_spiDevice.fileName();
    if (!m_spiDevice.exists()) {
        qCWarning(dcSpi()) << "Neuron SPI interface does not exist" << m_spiDevice.fileName();
        return false;
    }

    if (!m_spiDevice.open(QFile::ReadOnly)) {
        qCWarning(dcSpi()) << "Could not open SPI interface:" << m_spiDevice.fileName();
        return false;
    }
    return true;
}

void Spi::run()
{
    qCInfo(dcSpi()) << "SPI loop started for" << m_spiDevice.fileName();
    spi_ioc_transfer spiTransfer[7];
    int messageCount = 2;

    while (!isInterruptionRequested()) {

        if (!m_messageQueue.isEmpty()) {
            SpiMessage * const message = m_messageQueue.dequeue();
            message->startTimeoutTimer();

            if (message->messageLength() < 6) {
                qWarning(dcSpi()) << "Invalid SPI message, length must be min 6 bytes";
            } else if (message->messageLength() == 6) {
                // One phase operation
                messageCount = 2;
                memset(spiTransfer, 0, sizeof(spiTransfer));
                spiTransfer[0].delay_usecs = m_nssDefaultPause;
                spiTransfer[1].delay_usecs = 0;
                spiTransfer[1].tx_buf = (unsigned long) message->txData();
                spiTransfer[1].rx_buf = (unsigned long) message->rxData();
                spiTransfer[1].len = message->messageLength();

                // Sending data on the SPI bus
                if (ioctl(m_spiDevice.handle(), SPI_IOC_MESSAGE(2), &spiTransfer) < 1) {
                    qCWarning(dcSpi()) << "Can't send SPI message";
                    emit message->errorOccurred(SpiMessage::Error::UnknownError);
                }
                emit message->finished();
            } else {
                // Two phase operation
                memset(spiTransfer, 0, sizeof(spiTransfer));
                spiTransfer[0].delay_usecs = m_nssDefaultPause;    // starting pause between NSS and SCLK
                spiTransfer[1].tx_buf = (unsigned long) message->txData();
                spiTransfer[1].rx_buf = (unsigned long) message->rxData();
                spiTransfer[1].len = 6;

                int total = message->messageLength();
                messageCount = 2;
                // Splitting data up to fit into SPI messages
                while (total > 0 && messageCount < 7) {
                    spiTransfer[messageCount].tx_buf = (unsigned long)message->txData()+6+(m_maxSpiRx*(messageCount-2));
                    spiTransfer[messageCount].rx_buf = (unsigned long)message->rxData()+6+(m_maxSpiRx*(messageCount-2));
                    if ((total - m_maxSpiRx) > 0) {
                        spiTransfer[messageCount].len = m_maxSpiRx;
                        total -= m_maxSpiRx;
                    } else {
                        spiTransfer[messageCount].len = total;
                        total = 0;
                    }
                    messageCount++;
                }
                // Sending data on the SPI bus
                if (ioctl(m_spiDevice.handle(), SPI_IOC_MESSAGE(messageCount), spiTransfer) < 1) {
                    qCWarning(dcSpi()) << "Can't send SPI message";
                    emit messageSent(false, message);
                } else {
                    emit messageSent(true, message);
                }
            }
            QThread::msleep(1);
        } else {
            QThread::msleep(100);
        }
    }
}

bool Spi::setSpiSpeed(int speed)
{
    qCInfo(dcSpi()) << "Setting SPI speed to" << speed/1000000 << "MHz";;
    if (ioctl(m_spiDevice.handle(), SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        qCWarning(dcSpi) << "Cannot set speed" << m_spiDevice.errorString();
        return false;
    }
    return true;
}
/*
bool Spi::onePhaseOperation(SpiMessage *message)
{
}

    uint16_t crc = crcString((uint8_t*)&rx, m_sizeOfCommunicationHeader, 0);
    if (crc != rx.crc) {
        qCWarning(dcSpi()) << "Invalid CRC in one-phase operation";
        return rx;
    }

    if ((*((uint32_t*)&rx) & 0xffff00ff) == m_idlePattern) {
        return rx;
    }

    if (rx.op != FunctionCode::WriteCharacter) {
        qCWarning(dcSpi()) << "Unexpected reply in one-phase operation, Function code" << FunctionCode(rx.op) << QString("Length 0x%1, Register 0x%2").arg(rx.len, 0, 16).arg(rx.reg, 0, 16);
        return rx;
    }
    return rx;
}

bool Spi::twoPhaseOperation(SpiMessage *message)
{
    uint16_t tr_len2;
    uint16_t crc;

    // Prepare transactional structure
    spi_ioc_transfer spiTransfer[7];

    crc = NeuronUtil::crcString((uint8_t*)&m_rx1, m_sizeOfCommunicationHeader, 0);
    if (crc != m_rx1.crc) {
        qCWarning(dcSpi()) << "Bad 1.crc in two phase operation";
        return false;
    }

    crc = crcString(m_rx2, tr_len2, crc);

    if (m_rx1.op == FunctionCode::WriteCharacter) {
        if (((uint16_t*)m_rx2)[tr_len2>>1] != crc) {
            qCWarning(dcSpi()) << "Bad 2.crc in two phase operation";
            return false;
        }
        return true;
    }
    if (((uint16_t*)m_rx2)[tr_len2>>1] != crc) {
        qCWarning(dcSpi()) << "Bad 2.crc in two phase operation";
        return false;
    }
    if ((*((uint32_t*)&m_rx1) & 0xffff00ff) == m_idlePattern) {
        return true;
    }
    qCWarning(dcSpi()) << "Unexpected reply in two phase operation" << FunctionCode(m_rx1.op) << QString("Length 0x%1, Register 0x%2").arg(m_rx1.len, 0, 16).arg(m_rx1.reg, 0, 16);
    return false;
}
*/
void Spi::sendMessage(SpiMessage * const message)
{
    m_messageQueue.enqueue(message);
}
