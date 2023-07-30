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

#ifndef SPIMESSAGE_H
#define SPIMESSAGE_H

#include <QObject>
#include <QDebug>

#include "neurondefines.h"

class SpiMessage : public QObject
{
    Q_OBJECT
public:
    enum Error {
        NoError,
        TimeoutError,
        ProtocolError,
        UnknownError
    };
    explicit SpiMessage(QObject *parent = nullptr);
    // Constructor for read messages
    SpiMessage(FunctionCode functionCode, int address, int length, QObject *parent = nullptr);
    // Constructor for write messages
    SpiMessage(FunctionCode functionCode, int address, quint16 data, QObject *parent = nullptr);
    SpiMessage(FunctionCode functionCode, int address, const QVector<quint16> &data, QObject *parent = nullptr);

    void startTimeoutTimer();
    FunctionCode functionCode() const { return m_functionCode; }
    uint8_t length() const { return m_data.length(); }
    uint16_t address() const { return m_address; }
    uint8_t messageLength() const { return m_sizeOfNeuronSpiMessage; };
    bool checkRxCrc();

    uint8_t * txData() const { return m_tx; }
    uint8_t * rxData() { return m_rx; }

private:
    const int m_timeoutInterval = 100; // In milliseconds

    bool onePhaseOperation;
    FunctionCode m_functionCode = FunctionCode::Idle;
    uint16_t m_address = 0;
    uint8_t m_length = 0;
    QVector<quint16> m_data;


    typedef struct {
        uint8_t op;
        uint8_t len;
        uint16_t reg;
        uint16_t crc;
    } __attribute__((packed)) NeuronSpiMessage;
    const int m_sizeOfNeuronSpiMessage = 6;

    void setTxMessage();

    uint8_t *m_tx;
    uint8_t *m_rx;

    void timerEvent(QTimerEvent *event) override;

signals:
    void errorOccurred(Error error);
    void finished();
};


#endif // SPIMESSAGE_H
