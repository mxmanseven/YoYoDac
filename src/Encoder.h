#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

class Encoder {
    public:
    static int count;
    static int encoderAPin;
    static int encoderBPin;
    static int ledPin;
    
    static portMUX_TYPE mux;

    static void InitEncoder(
        int aPin,
        int bPin,
        int ledPinp
    );

    static void AttachInterrupts();
    static void DisableInterrupts();

    static int getCount();
    static void zeroCount();

    static void IRAM_ATTR isrA();
    static void IRAM_ATTR isrB();

    //private:
    Encoder(); // dont allow an instance of this class
};

#endif