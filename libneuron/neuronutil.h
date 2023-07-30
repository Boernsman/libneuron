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

#ifndef NEURONUTIL_H
#define NEURONUTIL_H

#include <QObject>

#define IS_CALIB(hw)  (((hw) & 0x8) != 0)
#define HW_BOARD(hw)  ((hw) >> 8)
#define HW_MAJOR(hw)  (((hw) & 0xf0) >> 4)
#define HW_MINOR(hw)  ((hw) & 0x07)

#define SW_MAJOR(sw)  ((sw) >> 8)
#define SW_MINOR(sw)  ((sw) & 0xff)

class NeuronUtil : public QObject
{
    Q_OBJECT
public:

    struct BoardVersion {
        uint16_t SwVersion;
        uint16_t HwVersion;
        uint16_t BaseHwVersion;
        uint8_t DiCount;
        uint8_t DoCount;
        uint8_t AiCount;
        uint8_t AoCount;
        uint8_t UartCount;
        // ------- not in register block 1000+
        uint16_t uLedCount;
        uint16_t IntMaskRegister;
    };

    explicit NeuronUtil(QObject *parent = nullptr);

    static BoardVersion parseVersion(uint16_t *register1000);
    static QString firmwareName(int hw_version, int hw_base, const char* fwdir, const char* ext);
    static void printUpboards(int filter);
    static int upboardExists(int board);
    static int checkCompatibility(int hw_base, int upboard);
    static int getBoardSpeed(const BoardVersion &boardVersion);
    static uint16_t crcString(uint8_t *inputstring, int length, uint16_t initval);
};

#endif // NEURONUTIL_H
