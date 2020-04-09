#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

class Encoder {
    public:
    static int count;
    static int encoderAPin;
    static int encoderBPin;
    static int ledPin;

    static void InitEncoder(
        int aPin,
        int bPin,
        int ledPinp
    );

    static void IRAM_ATTR isrA();

    //private:
    Encoder(); // dont allow an instance of this class
};

#endif