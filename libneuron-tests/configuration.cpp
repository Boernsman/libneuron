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

#include "configuration.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Configuration::Configuration(QObject *parent)
    : QObject{parent}
{

}

bool Configuration::loadTestConfigration(const QString &fileName)
{
    QFile *testConfigurationFile = new QFile(fileName);
    if (!testConfigurationFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not load file" << fileName << testConfigurationFile->errorString();
        testConfigurationFile->deleteLater();
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(testConfigurationFile->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Could not parse JSON file" << error.errorString();
        return false;
    }

    if (!doc.isArray()) {
        qWarning() << "Given JSON doc is not an array.";
        return false;
    }

    foreach (QJsonValue testValue, doc.array()) {
        QJsonObject object = testValue.toObject();
        qInfo() << "Loaded test from configuration:";
        TestDescriptor test;
        if (object.contains("type")) {
            QString value = object.value("type").toString();
            qInfo() << "    - type" << value;
            if (value == "digital_io_connection") {
                test.setTestType(TestDescriptor::TestType::DigitalIOConnection);
            } else if (value == "analog_io_connection") {
                test.setTestType(TestDescriptor::TestType::AnalogIOConnection);
            } else if (value == "digital_read_input") {
                test.setTestType(TestDescriptor::TestType::ReadDigitalInput);
            } else {
                qDebug() << "Unkown test type";
                continue;
            }
        } else {
            qInfo() << "   - Test type missing. Skipping test.";
        }
        if (object.contains("input_circuit")) {
            QString value = object.value("input_circuit").toString();
            qInfo() << "    - input" << value;
            if (!test.setInputCircuit(value)) {
                continue;
            }
        }
        if (object.contains("output_circuit")) {
            QString value = object.value("output_circuit").toString();
            qInfo() << "    - output" << value;
            if (!test.setOutputCircuit(value)) {
                continue;
            }
        }
        if (object.contains("interval")) {
            test.setInterval(object.value("interval").toInt());
        }
        if (object.contains("duration")) {
            test.setDuration(object.value("duration").toInt());
        }
        if (!test.isValid()) {
            qDebug() << "Test is not valid, skipping";
            continue;
        }
        m_testDescriptors.append(test);
    }
    return true;
}

QList<TestDescriptor> Configuration::testDesriptors()
{
    return m_testDescriptors;
}


bool TestDescriptor::isValid()
{
    switch (m_testType) {
    case TestDescriptor::TestType::ReadDigitalInput:
        return true;
    case TestDescriptor::TestType::DigitalIOConnection:
        return true;
    case TestDescriptor::TestType::AnalogIOConnection:
        return true;
    default:
        return false;
    }
}

bool TestDescriptor::setInputCircuit(const QString &inputCircuit)
{
    if (!checkCircuitFormat(inputCircuit)) {
        return false;
    }
    m_circuitInput = inputCircuit;
    return true;
}

bool TestDescriptor::setOutputCircuit(const QString &outputCircuit)
{
    if (!checkCircuitFormat(outputCircuit)) {
        return false;
    }
    m_circuitOutput = outputCircuit;
    return true;
}

bool TestDescriptor::setInterval(int ms)
{
    if (ms < 10 || ms > 10000) {
        qDebug() << "Interval must be between 10 and 10000";
        return false;
    }
    return true;
}

void TestDescriptor::setDuration(int ms)
{
    m_duration = ms;
}

bool TestDescriptor::checkCircuitFormat(const QString &circuit)
{
    if (circuit.length() < 3 || circuit.length() > 4) {
        return false;
    }
    if (circuit.at(1) != '.') {
        qDebug() << "Circuit must in format of X.Y, e.g. 1.2";
        return false;
    }
    return true;
}
