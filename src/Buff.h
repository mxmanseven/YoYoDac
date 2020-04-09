#ifndef BUFF_H
#define BUFF_H

#include <Arduino.h>

struct Sample {
    uint32_t sample;
    uint32_t mills;
};

class Buff
{
    public:
    Buff();
    int Push(Sample value, bool& isFulll);
    Sample GetNext(bool& isMore);
    static const int buffSize = 1024;
    Sample aBuff[buffSize];
    Sample bBuff[buffSize];

    int aHead;
    int aTail;
    bool aIsFull;
    int bHead;
    int bTail;
    bool bIsFull;
};

#endif