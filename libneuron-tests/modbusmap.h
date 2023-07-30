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

#ifndef MODBUSMAP_H
#define MODBUSMAP_H

#include <QObject>
#include <QHash>

class RegisterDescriptor;

class ModbusMap : public QObject
{
    Q_OBJECT
public:
    explicit ModbusMap(const QString &neuronModel, QObject *parent = nullptr);

    int numberOfNodes();
    bool loadModbusMap();

    QHash<QString, RegisterDescriptor> relayOutputRegisters();
    QHash<QString, RegisterDescriptor> digitalOutputRegisters();
    QHash<QString, RegisterDescriptor> digitalInputRegisters();
    QHash<QString, RegisterDescriptor> userLEDRegisters();
    QHash<QString, RegisterDescriptor> analogInputRegisters();
    QHash<QString, RegisterDescriptor> analogOutputRegisters();

private:
    QString m_model;

    QHash<QString, RegisterDescriptor> m_modbusRelayOutputRegisters;
    QHash<QString, RegisterDescriptor> m_modbusDigitalOutputRegisters;
    QHash<QString, RegisterDescriptor> m_modbusDigitalInputRegisters;
    QHash<QString, RegisterDescriptor> m_modbusUserLEDRegisters;
    QHash<QString, RegisterDescriptor> m_modbusAnalogInputRegisters;
    QHash<QString, RegisterDescriptor> m_modbusAnalogOutputRegisters;

    RegisterDescriptor registerDescriptorFromStringList(const QStringList &data);
};

class RegisterDescriptor
{
public:
    enum RWPermission {
        RWPermissionNone,
        RWPermissionRead,
        RWPermissionReadWrite,
        RWPermissionWrite
    };
    RegisterDescriptor() {}
    RegisterDescriptor(int subNode, int address, int count = 1, RWPermission readWrite = RWPermissionRead) :
        m_address(address),
        m_subNode(subNode),
        m_count(count),
        m_readWrite(readWrite)
    {};

    int address() { return m_address; }
    void setAddress(int address) { m_address = address; }
    int subNode() { return m_subNode; }
    void setSubNode(int subNode) { m_subNode = subNode; }
    int count() { return m_count; }
    void setCount(int count) { m_count = count; }
    RWPermission permission() { return m_readWrite; }
    void setPermission(RWPermission readWrite) { m_readWrite = readWrite; }

private:
    int m_address;
    int m_subNode; // 1 to 3
    int m_count = 1;
    RWPermission m_readWrite;
};

#endif // MODBUSMAP_H
