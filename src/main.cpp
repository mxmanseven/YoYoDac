#include <Arduino.h>

#include "Buff.h"
#include "Encoder.h"
#include "FileKnh.h"

const int LED_BUILTIN_PIN = 2; // non-cam
//const int LED_BUILTIN_PIN = 33; // cam

const int LED_CAMERA_BUILTIN_PIN = 4;
const byte encoderPinA = 27;
const byte encoderPinB = 26;

// cam ish:
// need some more free pins for get working. can use pins dedicated to the camera wiht a fpc breakout.
// const byte encoderPinA = 12; // 12 is used for the sd card and cannot also be used for the encoder.
// const byte encoderPinB = 16;

volatile uint32_t count = 0;
uint32_t protectedCount = 0;
uint32_t previousCount = 0;

const char* FILE_PATH = "/data.txt";

Buff buff;
FileKnh fileKnh;

void initEncoder() {
  Encoder::InitEncoder(encoderPinA, encoderPinB, LED_BUILTIN_PIN);
}

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool isBuffFull = false;
volatile bool timerTriggered = false;

volatile int timerExpCunt = 0;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  timerExpCunt++;
  Sample s;
  s.sample = Encoder::getCount();
  s.mills = millis();
  bool isFullLoc = false;
  buff.Push(s, isFullLoc);
  if (isFullLoc) isBuffFull = true;

  timerTriggered = true;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void initTimer() {
  // https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
  int timerHdId = 0;
  int timerPrescaller = 80;
  timer = timerBegin(timerHdId, timerPrescaller, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 10000, true);
  timerAlarmEnable(timer);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("in setup");

   pinMode(LED_BUILTIN_PIN, OUTPUT);
  // pinMode(LED_CAMERA_BUILTIN_PIN, OUTPUT);

   digitalWrite(LED_BUILTIN_PIN, LOW);
  // digitalWrite(LED_CAMERA_BUILTIN_PIN, LOW);

   initEncoder();

  fileKnh.initSdFs();
  fileKnh.writeFile(SD, FILE_PATH, "");

  initTimer();

  delay(2000);
  Serial.println("exiting setup");
}

void loop() {
  bool isBuffFullLoc = false;
  int timerExeCountLoc = 0;

  delay(1000);
  Serial.println(millis());


 digitalWrite(LED_BUILTIN_PIN, HIGH);
 delay(1000);
 digitalWrite(LED_BUILTIN_PIN, LOW);

 bool isMore = false;
 Sample s = buff.GetNext(isMore);
  Serial.println(s.sample);

  portENTER_CRITICAL_ISR(&timerMux);
  isBuffFullLoc = isBuffFull;
  timerExeCountLoc = timerExpCunt;
  portEXIT_CRITICAL_ISR(&timerMux);

  Serial.println("timer exec count " + String(timerExeCountLoc));

  if (isBuffFullLoc) {
  // if (isFull) {
    Serial.println("Buff is full, writing to file, ms: " + String(millis()));
    fileKnh.appendBuffToFile(SD, FILE_PATH, buff);

    portENTER_CRITICAL_ISR(&timerMux);
    isBuffFull = false;
    portEXIT_CRITICAL_ISR(&timerMux);
  }
}