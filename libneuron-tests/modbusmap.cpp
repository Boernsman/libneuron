// MIT Licen"e
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

#include "modbusmap.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

ModbusMap::ModbusMap(const QString &neuronModel, QObject *parent)
    : QObject{parent},
      m_model(neuronModel)
{

}

int ModbusMap::numberOfNodes()
{
    if (m_model.startsWith('S')) {
        return 1;
    } else if (m_model.startsWith('M')) {
        return 2;
    } else if (m_model.startsWith('L')) {
        return 3;
    }
    return 0;
}


bool ModbusMap::loadModbusMap()
{
    qDebug() << "Load modbus map";

    QStringList fileCoilList;
    QStringList fileRegisterList;

    int subUnits = numberOfNodes();

    if (subUnits < 1) {
        qDebug() << "Unknown Neuron model";
        return false;
    }

    QString mainDir;
    {
        const QString relativePath = "./modbus_maps/";
        if (QDir(relativePath).exists()) {
            mainDir = relativePath;
        } else {
            qDebug() << "Could not find modbus maps at relative path:" << mainDir;
            const QString installationPath = "/usr/share/libneuron/maps/";
            mainDir = installationPath;
            if (!QFile(mainDir).exists()) {
                qDebug() << "Could not find modbus maps at installation path:" << mainDir;
                return false;
            }
        }
    }

    for(int i = 1; i <= subUnits; i++) {
        const QString fileName = QString("Neuron_%1/Neuron_%1-Coils-group-%2.csv").arg(m_model).arg(i, 0, 10);
        QString path = mainDir + fileName;
        qDebug() << "Open CSV File:" << path;
        QFile *csvFile = new QFile(path);
        if (!csvFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << csvFile->errorString();
            csvFile->deleteLater();
            return false;
        }
        QTextStream *textStream = new QTextStream(csvFile);
        while (!textStream->atEnd()) {
            QString line = textStream->readLine();
            QStringList list = line.split(',');
            if (list.length() <= 4) {
                qWarning() << "Corrupted CSV file:" << csvFile->fileName();
                csvFile->deleteLater();
                return false;
            }
            if (list[4] == "Basic") {
                QString circuit = list[3].split(" ").last();
                if (list[3].contains("Digital Input", Qt::CaseSensitivity::CaseInsensitive)) {
                    m_modbusDigitalInputRegisters.insert(circuit, RegisterDescriptor(i, list[1].toInt()));
                    qDebug() << "Found input register. Curcuit name:" << circuit << "Node" << i << "Register:" << list[1].toInt();
                } else if (list[3].contains("Digital Output", Qt::CaseSensitivity::CaseInsensitive)) {
                    m_modbusDigitalOutputRegisters.insert(circuit, RegisterDescriptor(i, list[1].toInt()));
                    qDebug() << "Found output register. Circuit name:" << circuit << "Node" << i << "Register:"  << list[1].toInt();
                } else if (list[3].contains("Relay Output", Qt::CaseSensitivity::CaseInsensitive)) {
                    m_modbusRelayOutputRegisters.insert(circuit, RegisterDescriptor(i, list[1].toInt()));
                    qDebug() << "Found relay register. Circuit name:" << circuit << "Node" << i << "Register:" << list[1].toInt();
                } else if (list[3].contains("User Programmable LED", Qt::CaseSensitivity::CaseInsensitive)) {
                    m_modbusUserLEDRegisters.insert(circuit, RegisterDescriptor(i, list[1].toInt()));
                    qDebug() << "Found user programmable led. Circuit name:" << circuit << "Node" << i << "Register:" << list[1].toInt();
                }
            }
        }
        csvFile->close();
    }


    for(int i = 1; i <= subUnits; i++) {
        const QString fileName = QString("Neuron_%1/Neuron_%1-Registers-group-%2.csv").arg(m_model).arg(i, 0, 10);
        QString path = mainDir + fileName;
        qDebug() << "Open CSV File:" << path;
        QFile *csvFile = new QFile(path);
        if (!csvFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << csvFile->errorString();
            csvFile->deleteLater();
            return false;
        }
        QTextStream *textStream = new QTextStream(csvFile);
        while (!textStream->atEnd()) {
            QString line = textStream->readLine();
            QStringList list = line.split(',');
            if (list.length() <= 5) {
                qWarning() << "Corrupted CSV file:" << csvFile->fileName();
                csvFile->deleteLater();
                return false;
            }
            if (list.last() == "Basic") {
                QString circuit = list[3].split(" ").last();
                if (list[5].contains("Analog Input Value", Qt::CaseSensitivity::CaseInsensitive)) {
                    m_modbusAnalogInputRegisters.insert(circuit, registerDescriptorFromStringList(list));
                    qDebug() << "Found analog input:" << circuit;
                } else if (list[5].contains("Analog Output Value", Qt::CaseSensitivity::CaseInsensitive)) {
                    m_modbusAnalogOutputRegisters.insert(circuit, registerDescriptorFromStringList(list));
                    qDebug() << "Found analog output:" << circuit;
                }
            }
        }
        csvFile->close();
        csvFile->deleteLater();
    }
    return true;
}

QHash<QString, RegisterDescriptor> ModbusMap::relayOutputRegisters()
{
    return m_modbusRelayOutputRegisters;
}

QHash<QString, RegisterDescriptor> ModbusMap::digitalOutputRegisters()
{
    return m_modbusDigitalOutputRegisters;
}

QHash<QString, RegisterDescriptor> ModbusMap::digitalInputRegisters()
{
    return m_modbusDigitalInputRegisters;
}

QHash<QString, RegisterDescriptor> ModbusMap::userLEDRegisters()
{
    return m_modbusUserLEDRegisters;
}

QHash<QString, RegisterDescriptor> ModbusMap::analogInputRegisters()
{
    return m_modbusAnalogInputRegisters;
}

QHash<QString, RegisterDescriptor> ModbusMap::analogOutputRegisters()
{
    return m_modbusAnalogOutputRegisters;
}

RegisterDescriptor ModbusMap::registerDescriptorFromStringList(const QStringList &data)
{
    RegisterDescriptor descriptor;
    if (data.count() < 7) {
        return descriptor;
    }
    descriptor.setAddress(data[0].toInt());
    descriptor.setCount(data[2].toInt());
    if (data[3] == "RW") {
        descriptor.setPermission(RegisterDescriptor::RWPermissionReadWrite);
    } else if (data[3] == "W") {
        descriptor.setPermission(RegisterDescriptor::RWPermissionWrite);
    } else if (data[3] == "R") {
        descriptor.setPermission(RegisterDescriptor::RWPermissionRead);
    }

    //TODO
    //descriptor.setCircuit(data[5].split(" ").last());
    //descriptor.setCategory(data.last());

    /*if (data[5].contains("Analog Input Value", Qt::CaseSensitivity::CaseInsensitive)) {
        descriptor.registerType = QModbusDataUnit::RegisterType::InputRegisters;
    } else if (data[5].contains("Analog Output Value", Qt::CaseSensitivity::CaseInsensitive)) {
        descriptor.registerType = QModbusDataUnit::RegisterType::HoldingRegisters;
    }*/
    return descriptor;
}
