#ifndef BUFF_H
#define BUFF_H

#include <Arduino.h>

struct Sample {
    int32_t sample;
    uint32_t mills;
};

String SampleToString(Sample s);

class Buff
{
    public:
    Buff();
    int Push(Sample value, bool& isFulll);
    Sample GetNext(bool& isMore);
    void ZeroOut();
    void Test();
    static const int buffSize = 1024;
    // use a small buffer so that data is writen to disk quickly
    // to make debugging reading data from disk more easy
    //static const int buffSize = 10;
    Sample aBuff[buffSize];
    Sample bBuff[buffSize];

    int aHead;
    int aTail;
    bool aIsFull;
    int bHead;
    int bTail;
    bool bIsFull;
    Sample lastSample;
};

#endif