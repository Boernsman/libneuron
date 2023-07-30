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


#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QHash>
#include <QObject>

#include <neuronspi.h>

#include "testengine.h"
#include "configuration.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("libneuron tests");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Test the neuron SPI communication");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("model", "Neuron model type");

    QCommandLineOption fileOption(QStringList() << "f" << "file", "Test configuration <file>", "./test.json");
    parser.addOption(fileOption);

    QCommandLineOption switchAllOption(QStringList() << "a" << "all", "switch all <on/off>", "off");
    parser.addOption(switchAllOption);
    parser.process(app);

    QString testFile = parser.value(fileOption);



    TestEngine *testEngine = new TestEngine();
    qInfo() << "Given Neuron model type is" << parser.positionalArguments().first();
    if (!testEngine->loadMobusMap(parser.positionalArguments().first())) {
        qWarning() << "Could not load modbus map";
        return -1;
    }
    if (!testEngine->initHardware()) {
        qWarning() << "Could not init hardware";
        return -1;
    }

    QString allOnOff = parser.value(switchAllOption);
    if (!allOnOff.isEmpty()) {
        qDebug() << "Switch all option is set" << allOnOff;

        bool value;
        if (allOnOff == "on") {
            value = true;
        } else if (allOnOff == "off") {
            value = false;
        } else {
            qFatal("Illegal all option");
            return -1;
        }
        testEngine->setAllDigitalOutputs(value);
        testEngine->setAllRelayOutputs(value);
        testEngine->setAllUserLEDs(value);

        qInfo() << "Libneuron test were successfull";
        return 0;
    }

    Configuration config;
    if (!config.loadTestConfigration(parser.value(fileOption))) {
        qWarning() << "Could not read test configruation" << parser.value(fileOption);
        return -1;
    }

    QObject::connect(testEngine, &TestEngine::testsFinished, [=] () {
        qInfo() << "Libneuron test were successfull";
        return 0;
    });
    testEngine->start(&config);

    return app.exec();
}
