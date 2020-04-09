#include <Arduino.h>
#include "BluetoothSerial.h"
#include "Buff.h"
#include "Encoder.h"
#include "FileKnh.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Define CS pin for the SD card module
#define SD_CS_PIN 5

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

volatile int interruptCounter;
int totalInterruptCounter;
 
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
bool isFull = false;
bool timerTriggered = false;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  Sample s;
  s.sample = Encoder::getCount();
  s.mills = millis();
  bool isFullLoc = false;
  buff.Push(s, isFullLoc);
  if (isFullLoc) isFull = true;

  timerTriggered = true;
  portEXIT_CRITICAL_ISR(&timerMux); 
}

void initTimer() {
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

  fileKnh.initSdFs(SD_CS_PIN);
  fileKnh.writeFile(SD, FILE_PATH, "");

  initTimer();
}

void loop() {  
  if (isFull) {
    Serial.println("Buff is full, writing to file, ms: " + String(millis()));
    fileKnh.appendBuffToFile(SD, FILE_PATH, buff);
    isFull = false;
  }


  // Sample s;
  // s.sample = Encoder::getCount();

  // Serial.println(String(s.sample));

  // delay(1000);

  // if(timerTriggered) {
  //   timerTriggered = false;
  //    Serial.println(String(millis()) + " timer");
  // }

  // if (Serial.available()) {
  //   SerialBT.write(Serial.read());
  // }
  // if (SerialBT.available()) {
  //   Serial.write(SerialBT.read());
  // }
  // delay(2000);

  // logSDCard();
}