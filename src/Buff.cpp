#include "Buff.h"

String SampleToString(Sample sample) {
    return String(sample.mills)
      + "\t" + String(sample.sample)
      + "\r\n";
}

Buff::Buff() {
    aHead = 0;
    aTail = 0;
    aIsFull = false;
    bHead = 0;
    bTail = 0;
    bIsFull = false;
}

int Buff::Push(Sample value, bool& isFull) {
    lastSample = value;
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

void Buff::ZeroOut() {
    Sample value;
    value.mills = 0;
    value.sample = 0;
    lastSample = value;
    aHead = 0;
    aTail = 0;
    aIsFull = false;
    bHead = 0;
    bTail = 0;
    bIsFull = false;
    
    // maybe i don not need to zero out the array if the  pointers (ahead, atail...)
    // are zerowed.
    // for (int i = 0; i < buffSize; i++) {
    //     aBuff[i] =
    // }
}

// set loop count to many times buff size to exercise 
// moving between buffers.
// Should print out numbers 1 to loop count.
void Buff::Test() {
    Buff buff;
    Serial.println("Buff Test");

    for(int i = 0; i < 100; i++) {
        Sample sample;
        sample.mills = i;
        sample.sample = -1 * i;
        bool isFull = false;
        buff.Push(sample, isFull);
        
        if (isFull) {
            bool isMore = true;
            while (isMore) {
                sample = buff.GetNext(isMore);
                Serial.println(String(sample.mills));
            }
        }
    }
}