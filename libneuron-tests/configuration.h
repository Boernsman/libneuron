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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include <QHash>
#include <QPair>

class TestDescriptor;

class Configuration : public QObject
{
    Q_OBJECT
public:
    explicit Configuration(QObject *parent = nullptr);

    bool loadTestConfigration(const QString &fileName);

    QList<TestDescriptor> testDesriptors();

private:
    QList<TestDescriptor> m_testDescriptors;
};

class TestDescriptor
{
public:

    enum TestType {
        ReadDigitalInput,
        DigitalIOConnection,
        AnalogIOConnection,
    };

    explicit TestDescriptor() {};
    TestDescriptor(const QString &circuitInput, TestType testType = TestType::ReadDigitalInput, int duration = 0, int interval = 100);

    bool isValid();

    TestType testType() { return m_testType; }
    QString inputCircuit() { return m_circuitInput; }
    QString outputCircuit() { return m_circuitOutput; }
    int duration() { return m_duration; }
    int interval() { return m_interval; }

    void setTestType(TestType testType) { m_testType = testType; }
    bool setInputCircuit(const QString &inputCircuit);
    bool setOutputCircuit(const QString &outputCircuit);
    bool setInterval(int ms); // min 10 ms; max 10.000 ms
    void setDuration(int ms); // 0ms is infinite

private:
    TestType m_testType = TestType::ReadDigitalInput;
    QString m_circuitInput;
    QString m_circuitOutput;
    int m_duration = 0;
    int m_interval = 100;

    bool checkCircuitFormat(const QString &circuit);
};

#endif // CONFIGURATION_H
