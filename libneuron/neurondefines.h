#ifndef NEURONDEFINES_H
#define NEURONDEFINES_H

enum FunctionCode {
    Invalide = 0,
    ReadBit = 1, //Read discrete input
    ReadRegister = 4, //Input Registers
    WriteBit = 5, // Write Single Coil
    WriteRegister = 6,
    WriteBits = 15,
    WriteCharacter = 65, //Non modbus conform
    WriteString = 100, //Non modbus conform
    ReadString = 101, //Non modbus conform
    Idle = 0xfa //Non modbus conform
};

#endif // NEURONDEFINES_H
