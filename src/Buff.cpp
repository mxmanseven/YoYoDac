#include "Buff.h"

Buff::Buff() {
    aHead = 0;
    aTail = 0;
    aIsFull = false;
    bHead = 0;
    bTail = 0;
    bIsFull = false;
}

int Buff::Push(Sample value, bool& isFull) {
    int result = 0;
    if (!aIsFull && aHead < buffSize) {
        aBuff[aHead++] = value;
        if(aHead == buffSize) {
            aIsFull = true;
            isFull = true;
            aHead = 0;
            aTail = 0;
        }
    }    
    else if (!bIsFull && bHead < buffSize) {
        bBuff[bHead++] = value;
        if(bHead == buffSize) {
            bIsFull = true;
            isFull = true;
            bHead = 0;
            bTail = 0;
        }
    }
    else {
        isFull = true;
        result = -1;
    }
    return result;
}

Sample Buff::GetNext(bool& isMore) {
    isMore = false;
    Sample value;
    if (aIsFull) {
        value = aBuff[aTail++];
        if (aTail == buffSize) {
            aIsFull = false;
            aHead = 0;    
            aTail = 0;
        }
        else {
            isMore = true;
        }
    }    
    else if (bIsFull) {
        value = bBuff[bTail++];
        if (bTail == buffSize) {
            bIsFull = false;
            bHead = 0;    
            bTail = 0;
        }
        else {
            isMore = true;
        }
    }
    return value;
}