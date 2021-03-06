#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

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

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_COMMAND_MODE_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_SAMPLE_UUID       "cda2ac87-84a9-4cbc-8de8-f0285889a4e9"
                                    

BLECharacteristic *pCharacteristicCommandMode;
BLECharacteristic *pCharacteristicSample;

class CallbackCommandMode: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *characteristic) {
      std::string value = characteristic->getValue();

      if (value.length() > 0) {
        Serial.print("New command: ");
        char commandMode = 'a'; 
        for (int i = 0; i < value.length(); i++) {
          Serial.print(value[i]);
          if(i == 0) {
            commandMode = value[i];
          }
        }
        Serial.println("");

        // Zero out encoder
        if(commandMode == 'Z') {
          // 1 set sample count to zero
          // 2 empty buff
          // 3 clear file
          
          Encoder::zeroCount();
          buff.ZeroOut();
          fileKnh.deleteFile(SD, FILE_PATH);
        }

        // Last sample
        if(commandMode == 'L') {
          // get last sample
          // send to ble client

          // get buffer lock
          Sample lastSample = buff.lastSample;

          // knh todo test!
          std::string ss = std::string(
              (char*) SampleToString(lastSample).c_str(), 
              strlen(SampleToString(lastSample).c_str()));

          Serial.print("last sample ");
          Serial.println(SampleToString(lastSample));

          pCharacteristicSample->setValue(ss);
        }

        // Download file, one sample at a time. the last message will be empty.
        if(commandMode == 'D') {
          // set flag for main loop to do work
          // open file, send one sample at a time
          // last sample is empty to indicate download is done.  
        }

        Serial.println();
        Serial.println("*********");
      }
    }
};

class CallbackSample: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristicCommandMode) {
      std::string value = pCharacteristicCommandMode->getValue();

      if (value.length() > 0) {
        Serial.println("sample update");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};

void initBle() {
  Serial.println("1- Download and install an BLE scanner app in your phone");
  Serial.println("2- Scan for BLE devices in the app");
  Serial.println("3- Connect to MyESP32");
  Serial.println("4- Go to CUSTOM CHARACTERISTIC in CUSTOM SERVICE and write something");
  Serial.println("5- See the magic =) ):");

  BLEDevice::init("YoYo DAC");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristicCommandMode = pService->createCharacteristic(
                                         CHARACTERISTIC_COMMAND_MODE_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristicCommandMode->setCallbacks(new CallbackCommandMode());
  pCharacteristicCommandMode->setValue("command mode");
  
  pCharacteristicSample = pService->createCharacteristic(
                                         CHARACTERISTIC_SAMPLE_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristicCommandMode->setCallbacks(new CallbackSample());
  pCharacteristicSample->setValue("sample");

  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
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

  initBle();

  delay(2000);
  Serial.println("exiting setup");
}

void loop() {
  bool isBuffFullLoc = false;
  int timerExeCountLoc = 0;

  delay(1000);
//  Serial.println(millis());

  // uint32_t m = millis();
  // uint32_t m2 = m * 2;
  // Serial.println(m, HEX);
  // pCharacteristicCommandMode->setValue(m);
  // pCharacteristicSample->setValue(m2);


  digitalWrite(LED_BUILTIN_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN_PIN, LOW);

  portENTER_CRITICAL_ISR(&timerMux);
  isBuffFullLoc = isBuffFull;
  timerExeCountLoc = timerExpCunt;
  portEXIT_CRITICAL_ISR(&timerMux);

  Serial.println("timer exec count " + String(timerExeCountLoc));

  if (isBuffFullLoc) {
    Serial.println("Buff is full, writing to file, ms: " + String(millis()));
    fileKnh.appendBuffToFile(SD, FILE_PATH, buff);

    portENTER_CRITICAL_ISR(&timerMux);
    isBuffFull = false;
    portEXIT_CRITICAL_ISR(&timerMux);
  }
}