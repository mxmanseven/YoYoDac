#include "Pins_Arduino.h"
#include "Encoder.h"

int Encoder::count = 0;
int Encoder::encoderAPin = 0;
int Encoder::encoderBPin = 0;
int Encoder::ledPin = 0;

portMUX_TYPE Encoder::mux = portMUX_INITIALIZER_UNLOCKED;

// rotory a chanel
void IRAM_ATTR Encoder::isrA() {
  int a = digitalRead(Encoder::encoderAPin);
  int b = digitalRead(Encoder::encoderBPin);

  portENTER_CRITICAL_ISR(&Encoder::mux);
  if(a != b) {
    Encoder::count ++;
    digitalWrite(Encoder::ledPin, HIGH);
  } else {
    Encoder::count --;
    digitalWrite(Encoder::ledPin, LOW);
  }
  portEXIT_CRITICAL_ISR(&Encoder::mux);
}

// rotory b chanel
void IRAM_ATTR Encoder::isrB() {
  int a = digitalRead(Encoder::encoderAPin);
  int b = digitalRead(Encoder::encoderBPin);

  portENTER_CRITICAL_ISR(&Encoder::mux);
  if(a == b) {
    Encoder::count ++;
    digitalWrite(Encoder::ledPin, HIGH);
  } else {
    Encoder::count --;
    digitalWrite(Encoder::ledPin, LOW);
  }
  portEXIT_CRITICAL_ISR(&Encoder::mux);
}

void Encoder::InitEncoder(
        int aPin,
        int bPin,
        int ledPinp) {
  count = 0;    
  Encoder::encoderAPin = aPin;
  Encoder::encoderBPin = bPin;
  Encoder::ledPin = ledPinp;

  pinMode(ledPinp, OUTPUT);
  digitalWrite(ledPinp, LOW);

  pinMode(Encoder::encoderAPin, INPUT_PULLUP);
  pinMode(Encoder::encoderBPin, INPUT_PULLUP);

  AttachInterrupts();
}

void Encoder::AttachInterrupts() {
  attachInterrupt(digitalPinToInterrupt(Encoder::encoderAPin), Encoder::isrA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(Encoder::encoderBPin), Encoder::isrB, CHANGE);
  // dissable one of the interrupts to reduce the resolution by half
  // with the amazon ldp3806-600bn this leads to about 1200 ticks per rotation - more than enough
}

void Encoder::DisableInterrupts() {
  detachInterrupt(digitalPinToInterrupt(Encoder::encoderAPin));
  detachInterrupt(digitalPinToInterrupt(Encoder::encoderBPin));
}

int Encoder::getCount() {
  // knh todo - add mutex or other isr protection
  int c = 0;
  portENTER_CRITICAL_ISR(&Encoder::mux);
  c = Encoder::count;
  portEXIT_CRITICAL_ISR(&Encoder::mux);
  return c;
}

void Encoder::zeroCount() {
  Encoder::count = 0;
}