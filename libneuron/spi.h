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

#ifndef SPI_H
#define SPI_H

#include <QThread>
#include <QObject>
#include <QFile>
#include <QDebug>
#include <QQueue>
#include <QLoggingCategory>

extern "C" {
#include <linux/spi/spidev.h>
}

#include "spimessage.h"

Q_DECLARE_LOGGING_CATEGORY(dcSpi)

class Spi : public QThread
{
    Q_OBJECT
public:
    explicit Spi(const QString &spiDevicePath, QObject *parent = nullptr);

    bool init();
    void run() override;

    bool setSpiSpeed(int speed);
private:

    QFile m_spiDevice;

    const int m_maxSpiRx = 64; // On the RPI 2,3 the SPI transmit is limitted to 94 bytes.
    const int m_maxSpiStr = 240;
    const int m_nssDefaultPause = 10;
    const uint32_t m_idlePattern = 0x0e5500fa;

    QQueue<SpiMessage *> m_messageQueue;

public slots:
    void sendMessage(SpiMessage * const message);

signals:
    void messageSent(bool success, SpiMessage * const message);
};


#endif // SPI_H
