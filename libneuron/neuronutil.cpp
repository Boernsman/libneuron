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

#include "neuronutil.h"

NeuronUtil::NeuronUtil(QObject *parent)
    : QObject{parent}
{

}

NeuronUtil::BoardVersion NeuronUtil::parseVersion(uint16_t *register1000)
{
    BoardVersion boardVersion;
    boardVersion.SwVersion = register1000[0];
    boardVersion.HwVersion = register1000[3];
    boardVersion.BaseHwVersion = register1000[4];

    boardVersion.DiCount = (register1000[1]) >> 8;
    boardVersion.DoCount = (register1000[1] & 0xff);
    boardVersion.AiCount = (register1000[2]) >> 8;
    boardVersion.AoCount = (register1000[2] & 0xff) >> 4;
    boardVersion.UartCount = (register1000[2] & 0x0f);
    boardVersion.uLedCount = 0;
    boardVersion.IntMaskRegister = 1007;

    if (SW_MAJOR(boardVersion.SwVersion) < 4) {
        boardVersion.HwVersion = (SW_MINOR(boardVersion.SwVersion) & 0xff) << 4 \
                                                                              | (SW_MINOR(boardVersion.SwVersion) & 0x0f);
        boardVersion.SwVersion = boardVersion.SwVersion & 0xff00;
        boardVersion.IntMaskRegister = 1003;
    } else {
        if ((boardVersion.SwVersion < 0x0403)) {  // developer version
            boardVersion.IntMaskRegister = 1004;
        }
        if (HW_BOARD(boardVersion.HwVersion) == 0) {
            if (boardVersion.SwVersion != 0x0400)
                boardVersion.uLedCount = 4;
        }
    }
    if ((HW_BOARD(boardVersion.BaseHwVersion) == 0x0b) && (HW_MAJOR(boardVersion.BaseHwVersion) <= 1))
        boardVersion.IntMaskRegister = 0;   // 4Ai4Ao has no interrupt

    return boardVersion;
}

int NeuronUtil::getBoardSpeed(const BoardVersion &boardVersion)
{
    // E-4Ai4Ao* used digital isolator on the SPI bus, leading to a reduced speed of 8MHz max.
    if (HW_BOARD(boardVersion.BaseHwVersion) == 11)
        return 8000000;

    // The default speed is 12MHz
    return 12000000;
}

const uint16_t CRC16TABLE[] = {
    0,  1408,  3968,  2560,  7040,  7680,  5120,  4480, 13184, 13824, 15360,
    14720, 10240, 11648, 10112,  8704, 25472, 26112, 27648, 27008, 30720, 32128,
    30592, 29184, 20480, 21888, 24448, 23040, 19328, 19968, 17408, 16768, 50048,
    50688, 52224, 51584, 55296, 56704, 55168, 53760, 61440, 62848, 65408, 64000,
    60288, 60928, 58368, 57728, 40960, 42368, 44928, 43520, 48000, 48640, 46080,
    45440, 37760, 38400, 39936, 39296, 34816, 36224, 34688, 33280, 33665, 34305,
    35841, 35201, 38913, 40321, 38785, 37377, 45057, 46465, 49025, 47617, 43905,
    44545, 41985, 41345, 57345, 58753, 61313, 59905, 64385, 65025, 62465, 61825,
    54145, 54785, 56321, 55681, 51201, 52609, 51073, 49665, 16385, 17793, 20353,
    18945, 23425, 24065, 21505, 20865, 29569, 30209, 31745, 31105, 26625, 28033,
    26497, 25089,  9089,  9729, 11265, 10625, 14337, 15745, 14209, 12801,  4097,
    5505,  8065,  6657,  2945,  3585,  1025,   385,   899,  1539,  3075,  2435,
    6147,  7555,  6019,  4611, 12291, 13699, 16259, 14851, 11139, 11779,  9219,
    8579, 24579, 25987, 28547, 27139, 31619, 32259, 29699, 29059, 21379, 22019,
    23555, 22915, 18435, 19843, 18307, 16899, 49155, 50563, 53123, 51715, 56195,
    56835, 54275, 53635, 62339, 62979, 64515, 63875, 59395, 60803, 59267, 57859,
    41859, 42499, 44035, 43395, 47107, 48515, 46979, 45571, 36867, 38275, 40835,
    39427, 35715, 36355, 33795, 33155, 32770, 34178, 36738, 35330, 39810, 40450,
    37890, 37250, 45954, 46594, 48130, 47490, 43010, 44418, 42882, 41474, 58242,
    58882, 60418, 59778, 63490, 64898, 63362, 61954, 53250, 54658, 57218, 55810,
    52098, 52738, 50178, 49538, 17282, 17922, 19458, 18818, 22530, 23938, 22402,
    20994, 28674, 30082, 32642, 31234, 27522, 28162, 25602, 24962,  8194,  9602,
    12162, 10754, 15234, 15874, 13314, 12674,  4994,  5634,  7170,  6530,  2050,
    3458,  1922,   514
};

uint16_t NeuronUtil::crcString(uint8_t *inputstring, int length, uint16_t initval)
{
    int i;
    uint16_t result = initval;
    for (i=0; i<length; i++) {
        result = (result >> 8) ^ CRC16TABLE[(result ^ inputstring[i]) & 0xff];
    }
    return result;
}
