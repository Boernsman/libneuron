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

#ifndef TESTENGINE_H
#define TESTENGINE_H

#include <QObject>
#include <QTimer>
#include <QVariant>
#include <QUuid>

#include "configuration.h"
#include "modbusmap.h"
#include "neuronspi.h"

class Test;

class TestEngine : public QObject
{
    Q_OBJECT
public:
    explicit TestEngine(QObject *parent = nullptr);

    bool initHardware();
    void setAllDigitalOutputs(bool value);
    void setAllRelayOutputs(bool value);
    void setAllUserLEDs(bool value);
    void start(Configuration *config);

    bool loadMobusMap(const QString &neuronModel);
private:
    QList<NeuronSpi *> m_spiList;

    ModbusMap *m_modbusMap;
    QList<Test *> m_tests;

private slots:
    void onTestError(const QUuid &testId, const QString &errorString);

    void onSetDigtialOutput(const QString &output, bool value);
    void onSetAnalogOutput(const QString &output, double value);

    bool onReadDigitalInput(const QString &inputCircuit);
    bool onReadDigitalOutput(const QString &outputCircuit);

    bool onReadAnalogInput(const QString &inputCircuit);
    bool onReadAnalogOutput(const QString &outputCircuit);

signals:
    void testsFinished();
};

class Test : public QObject
{
    Q_OBJECT
public:
    explicit Test(TestDescriptor testDescriptor, QObject *parent = nullptr);

private:
    QUuid m_testId;
    TestDescriptor m_testDescriptor;
    QTimer *m_intervalTimer;
    bool m_lastDigitalValue = false;
    double m_lastAnalogValue = 0.00;

private slots:
    void action();

signals:
    void testError(const QUuid &testId, const QString &errorString);

    void setDigitalOutput(const QString &output, bool value);
    void setAnalogOutput(const QString &output, double value);

    bool readDigitalInput(const QString &outputCircuit);
    bool readDigitalOutput(const QString &outputCircuit);

    bool readAnalogInput(const QString &outputCircuit);
    bool readAnalogOutput(const QString &outputCircuit);
};

#endif // TESTENGINE_H
