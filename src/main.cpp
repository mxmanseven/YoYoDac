#include <Arduino.h>
#include "BluetoothSerial.h"
#include "Buff.h"
#include "Encoder.h"
#include "FileKnh.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

const int LED_BUILTIN_PIN = 2;
const byte encoderPinA = 27;
const byte encoderPinB = 26;

volatile uint32_t count = 0;
uint32_t protectedCount = 0;
uint32_t previousCount = 0;

const char* FILE_PATH = "/data.txt";

BluetoothSerial SerialBT;
Buff buff;
FileKnh fileKnh;

void initBlueTooth() {  
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth! knh");
}

void initEncoder() {
  Encoder::InitEncoder(encoderPinA, encoderPinB, LED_BUILTIN_PIN);
}
 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
volatile bool isBuffFull = false;
volatile bool timerTriggered = false;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
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
  delay(1000);
  pinMode (LED_BUILTIN_PIN, OUTPUT);
  initEncoder();
  //initBlueTooth();

  fileKnh.initSdFs();
  fileKnh.writeFile(SD_MMC, FILE_PATH, "");

  initTimer();
}

void loop() {
  bool isBuffFullLoc = false;
  
  portENTER_CRITICAL_ISR(&timerMux);
  isBuffFullLoc = isBuffFull;
  portEXIT_CRITICAL_ISR(&timerMux); 

  if (isBuffFullLoc) {
  // if (isFull) {
    Serial.println("Buff is full, writing to file, ms: " + String(millis()));
    fileKnh.appendBuffToFile(SD_MMC, FILE_PATH, buff);
      
    portENTER_CRITICAL_ISR(&timerMux);
    isBuffFull = false;
    portEXIT_CRITICAL_ISR(&timerMux); 
   }
}