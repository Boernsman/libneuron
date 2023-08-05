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

#include "testengine.h"

#include <QTimer>
#include <QDebug>
#include <QtTest/QTest>

TestEngine::TestEngine(QObject *parent) :
    QObject{parent}
{

}

bool TestEngine::loadMobusMap(const QString &neuronModel)
{
    m_modbusMap = new ModbusMap(neuronModel, this);
    return m_modbusMap->loadModbusMap();
}

bool TestEngine::initHardware()
{
    int subNodes = m_modbusMap->numberOfNodes();

    for (int i=0; i< subNodes; i++) {
        NeuronSpi *spi = new NeuronSpi(i);
        qDebug() << "Init SPI" << i;
        if (!spi->init()) {
            qWarning() << "Could not init SPI";
            return false;
        }

        if (!spi->idleOperation()) {
            qWarning() << "Could not send idle operation message";
            return false;
        }
        m_spiList.append(spi);
    }
    return true;
}

void TestEngine::setAllDigitalOutputs(bool value)
{
    foreach (RegisterDescriptor reg, m_modbusMap->digitalOutputRegisters().values()) {
        auto spi = m_spiList.at(reg.subNode()-1);
        spi->writeBit(reg.address(), value);
        QTest::qWait(1); //Wait for 10 ms
    }
}

void TestEngine::setAllRelayOutputs(bool value)
{
    foreach (RegisterDescriptor reg, m_modbusMap->relayOutputRegisters().values()) {
        auto spi = m_spiList.at(reg.subNode()-1);
        spi->writeBit(reg.address(), value);
        QTest::qWait(1); //Wait for 10 ms
    }
}

void TestEngine::setAllUserLEDs(bool value)
{
    foreach (RegisterDescriptor reg, m_modbusMap->userLEDRegisters().values()) {
        auto spi = m_spiList.at(reg.subNode()-1);
        spi->writeBit(reg.address(), value);
        QTest::qWait(1); //Wait for 10 ms
    }
}


void TestEngine::start(Configuration *config)
{
    foreach (TestDescriptor testDescriptor, config->testDesriptors()) {

        Test *test = new Test(testDescriptor, this);
        m_tests.append(test);

        connect(test, &Test::setDigitalOutput, this, &TestEngine::onSetDigtialOutput);
        connect(test, &Test::setAnalogOutput, this, &TestEngine::onSetAnalogOutput);
        connect(test, &Test::readDigitalInput, this, &TestEngine::onReadDigitalInput);
        connect(test, &Test::readDigitalOutput, this, &TestEngine::onReadDigitalOutput);
        connect(test, &Test::destroyed, this, [this, test] {
            m_tests.removeAll(test);

            if (m_tests.isEmpty()) {
                emit testsFinished();
            }
        });
    }
}

void TestEngine::onTestError(const QUuid &testId, const QString &errorString)
{
    qDebug() << testId << errorString;
}

void TestEngine::onSetDigtialOutput(const QString &output, bool value)
{
    if(!m_modbusMap->relayOutputRegisters().contains(output)) {
        qWarning() << "Digital output does not exist:" << output;
        return;
    }
    auto reg = m_modbusMap->relayOutputRegisters().value(output);
    if (m_spiList.at(reg.subNode()-1) == nullptr) {
        qWarning() << "Subnote" << reg.subNode() << "does not exist";
        return;
    }
    qDebug() << "Write bit. Subnode" << reg.subNode() << "address" << reg.address() << "value" << value;
    m_spiList.at(reg.subNode()-1)->writeBit(reg.address(), value);
}

void TestEngine::onSetAnalogOutput(const QString &output, double value)
{
    if(!m_modbusMap->analogOutputRegisters().contains(output)) {
        qWarning() << "Analog output does not exist:" << output;
        return;
    }
    auto reg = m_modbusMap->analogOutputRegisters().value(output);
    if (m_spiList.at(reg.subNode()-1) == nullptr) {
        qWarning() << "Subnote" << reg.subNode() << "does not exist";
        return;
    }
    uint16_t regValue[2];
    regValue[0] = ((uint32_t)value >> 16);
    regValue[1] = ((uint32_t)value & 0xffff);
    m_spiList.at(reg.subNode()-1)->writeRegisters(reg.address(), 2, regValue);
}

bool TestEngine::onReadDigitalInput(const QString &inputCircuit)
{
    if(!m_modbusMap->digitalInputRegisters().contains(inputCircuit)) {
        qWarning() << "Analog input does not exist:" << inputCircuit;
        return false;
    }
    auto reg = m_modbusMap->digitalInputRegisters().value(inputCircuit);
    if (m_spiList.at(reg.subNode()-1) == nullptr) {
        qWarning() << "Subnote" << reg.subNode() << "does not exist";
        return false;
    }
    uint8_t value;
    m_spiList.at(reg.subNode()-1)->readBits(reg.address(), 1, &value);
    return (value > 0);
}

bool TestEngine::onReadDigitalOutput(const QString &outputCircuit)
{
    if(!m_modbusMap->digitalOutputRegisters().contains(outputCircuit)) {
        qWarning() << "Digital output does not exist:" << outputCircuit;
        return false;
    }

    auto reg = m_modbusMap->digitalOutputRegisters().value(outputCircuit);
    if (m_spiList.at(reg.subNode()-1) == nullptr) {
        qWarning() << "Subnote" << reg.subNode() << "does not exist";
        return false;
    }
    uint8_t value;
    m_spiList.at(reg.subNode()-1)->readBits(reg.address(), 1, &value);
    return (value > 0);
}

bool TestEngine::onReadAnalogInput(const QString &inputCircuit)
{
    if(!m_modbusMap->analogInputRegisters().contains(inputCircuit)) {
        qWarning() << "Analog input does not exist:" << inputCircuit;
        return false;
    }
    auto reg = m_modbusMap->analogOutputRegisters().value(inputCircuit);
    if (m_spiList.at(reg.subNode()-1) == nullptr) {
        qWarning() << "Subnote" << reg.subNode() << "does not exist";
        return false;
    }

    auto reply = m_spiList.at(reg.subNode()-1)->readRegisters(reg.address(), 2);
    connect(reply, &SpiReply::finished, reply, &SpiReply::deleteLater);
    connect(reply, &SpiReply::finished, this, [=] {
        //uint16_t regValue[2];
        //((regValue[1]<<16) & regValue[0]);
    });
    return true;
}

bool TestEngine::onReadAnalogOutput(const QString &outputCircuit)
{
    if(!m_modbusMap->analogOutputRegisters().contains(outputCircuit)) {
        qWarning() << "Analog output does not exist:" << outputCircuit;
        return false;
    }
    auto reg = m_modbusMap->analogOutputRegisters().value(outputCircuit);
    if (m_spiList.at(reg.subNode()-1) == nullptr) {
        qWarning() << "Subnote" << reg.subNode() << "does not exist";
        return false;
    }
    auto reply = m_spiList.at(reg.subNode()-1)->readRegisters(reg.address(), 2);
    connect(reply, &SpiReply::finished, reply, &SpiReply::deleteLater);
    connect(reply, &SpiReply::finished, this, [=] {
        //uint16_t regValue[2];
        //((regValue[1]<<16) & regValue[0]);
    });
    return true;
}

Test::Test(TestDescriptor testDescriptor, QObject *parent) :
    QObject{parent},
    m_testDescriptor(testDescriptor)
{
    m_testId = QUuid::createUuid();

    qDebug() << "Creating Test" <<  m_testId.toString(QUuid::WithoutBraces) << testDescriptor.testType() << testDescriptor.outputCircuit() << testDescriptor.inputCircuit() << testDescriptor.interval() << testDescriptor.duration();
    if (m_testDescriptor.duration() != 0) {
        QTimer::singleShot(m_testDescriptor.duration(), this, [this] {
            this->deleteLater();
        });
    }

    m_intervalTimer = new QTimer(this);
    connect(m_intervalTimer, &QTimer::timeout, this, &Test::action);
    m_intervalTimer->setSingleShot(false);
    m_intervalTimer->start(m_testDescriptor.interval());

}

void Test::action()
{
    switch (m_testDescriptor.testType()) {
    case TestDescriptor::TestType::DigitalIOConnection: {
        if (m_lastDigitalValue == true) {
            m_lastDigitalValue = false;
            emit setDigitalOutput(m_testDescriptor.outputCircuit(), false);
        } else {
            m_lastDigitalValue = true;
            emit setDigitalOutput(m_testDescriptor.outputCircuit(), true);
        }
        bool value = readDigitalInput(m_testDescriptor.inputCircuit());
        if (value != m_lastDigitalValue) {
            emit testError(m_testId, "Input value differs from output value");
        }
        break;
    }
    case TestDescriptor::TestType::AnalogIOConnection: {
        if (m_lastAnalogValue >= 10.0) {
            m_lastAnalogValue = 0.0;
        } else {
            m_lastAnalogValue += 0.5;
        }
        emit setAnalogOutput(m_testDescriptor.outputCircuit(), m_lastAnalogValue);
        //TODO read analog input and compare to output
        break;
    }
    case TestDescriptor::TestType::ReadDigitalInput: {
        readDigitalInput(m_testDescriptor.inputCircuit());
        break;
    }
    }
}
