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

#include "spimessage.h"

SpiMessage::SpiMessage(QObject *parent) :
    QObject{parent}
{

}

SpiMessage::SpiMessage(FunctionCode functionCode, int address, int length, QObject *parent) :
    QObject{parent},
    m_functionCode(functionCode),
    m_address(address),
    m_length(length)
{
    /*
     * Read message structure in bytes:
     *   Function
     *   Starting Address Hi
     *   Starting Address Lo
     *   Quantity of Registers Hi
     *   Quantity of Registers Lo
     *   Error Check Lo
     *   Error Check Hi
     */


    m_tx = new uint8_t[6];
    m_tx[0] = functionCode;
    m_tx[1] = address;
}



SpiMessage::SpiMessage(FunctionCode functionCode, int address, quint16 data, QObject *parent) :
    QObject{parent},
    m_functionCode(functionCode),
    m_address(address),
    m_data(data)
{
    m_length = 1;
}

SpiMessage::SpiMessage(FunctionCode functionCode, int address, const QVector<quint16> &data, QObject *parent) :
    QObject{parent},
    m_functionCode(functionCode),
    m_address(address),
    m_data(data)
{
    m_length = data.length();
}


bool SpiMessage::checkRxCrc()
{
    return true;
}

void SpiMessage::setTxMessage()
{
    if (m_functionCode == FunctionCode::WriteBit) {

    } else if (m_functionCode == FunctionCode::ReadBit || m_functionCode == FunctionCode::ReadRegister) {
        /*
         * Read bit message structure in bytes:
         *   Function
         *   Starting Address Hi
         *   Starting Address Lo
         *   Quantity of Registers Hi
         *   Quantity of Registers Lo
         *   Error Check Lo
         *   Error Check Hi
         */
        m_tx[0] = m_functionCode;
        m_tx[1] = (m_address >> 8) & 0xff;
        m_tx[2] = m_address & 0xff;
        m_tx[3] = (m_length >> 8) & 0xff;
        m_tx[4] = m_length & 0xff;
    }
}



SpiReply::SpiReply(QObject *parent) : QObject{parent}
{
    m_rx = new uint8_t[256+2+40];
}

bool SpiReply::isFinished() const
{
    return m_isFinished;
}

QVector<quint16> SpiReply::result() const
{
    return QVector<quint16>();
}

void SpiReply::setFinished(bool isFinished)
{
    m_isFinished = isFinished;
    if (isFinished) {
        emit finished();
    }
}

void SpiReply::setError(SpiError error, const QString &errorText)
{
    m_error = error;
    m_errorString = errorText;

    if (error != SpiError::NoError) {
        emit errorOccurred(error);
    }
}

void SpiReply::startTimeoutTimer()
{
    startTimer(m_timeoutInterval);
}

uint8_t *SpiReply::rxData()
{
    return m_rx;
}

void SpiReply::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    emit errorOccurred(SpiError::TimeoutError);
    emit finished();
}
