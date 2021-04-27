#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "Buff.h"
#include "Encoder.h"
#include "FileKnh.h"

const int LED_BUILTIN_PIN = 16;
const byte encoderPinA = 27;
const byte encoderPinB = 26;

const char* FILE_PATH = "/data.txt";

// when uploading file with BLE 
// avoid writing to file and buff
bool uploadFile = false;

// record samples at regular interval with a encoderTimer
hw_timer_t * encoderTimer = NULL;
portMUX_TYPE encoderTimerMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool isBuffFull = false;
volatile int timerExpCunt = 0;

Buff buff;
FileKnh fileKnh;

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
          portENTER_CRITICAL_ISR(&encoderTimerMux);
          Encoder::zeroCount();
          buff.ZeroOut();
          fileKnh.deleteFile(SD, FILE_PATH);
          portEXIT_CRITICAL_ISR(&encoderTimerMux);
        }

        // Last sample
        if(commandMode == 'L') {
          // get last sample
          // send to ble client

          portENTER_CRITICAL_ISR(&encoderTimerMux);
          Sample lastSample = buff.lastSample;
          portEXIT_CRITICAL_ISR(&encoderTimerMux);

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
          portENTER_CRITICAL_ISR(&encoderTimerMux);
          uploadFile = true;
          portEXIT_CRITICAL_ISR(&encoderTimerMux);
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

void initEncoder() {
  Encoder::InitEncoder(encoderPinA, encoderPinB, LED_BUILTIN_PIN);
}

void IRAM_ATTR onEncoderTimer() {
  portENTER_CRITICAL_ISR(&encoderTimerMux);
  if (uploadFile == true) return; // don't collect samples while downloading
  timerExpCunt++;
  Sample s;
  s.sample = Encoder::getCount();
  s.mills = millis();
  bool isFullLoc = false;
  buff.Push(s, isFullLoc);
  if (isFullLoc) isBuffFull = true;
  portEXIT_CRITICAL_ISR(&encoderTimerMux);
}

void initEncoderTimer() {
  // log encoder count every time this encoderTimer expires
  // instead of logging for each encoder change so that the log interval is consistant.
  // https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
  int timerHdId = 0;
  int timerPrescaller = 80;  // 80,000,000 base hz -> 1,000,000
  //int interruptAtScalledTickes = 5000; // 1,000,000 / 5,000 -> 200, some how this results in a timer each 5ms.
  int interruptAtScalledTickes = 10000; //  some how this results in a timer each 10ms, with prescaler of 80.
  //int interruptAtScalledTickes = 1000; //  some how this results in a timer each 1ms, with prescaler of 80.
  encoderTimer = timerBegin(timerHdId, timerPrescaller, true);
  timerAttachInterrupt(encoderTimer, &onEncoderTimer, true);
  timerAlarmWrite(encoderTimer, interruptAtScalledTickes, true);
  timerAlarmEnable(encoderTimer);
}

void setup() {
  Serial.begin(115200);
  delay(5000);

  Serial.println("in setup");

  pinMode(LED_BUILTIN_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN_PIN, LOW);

  initEncoder();

  fileKnh.initSdFs();
  fileKnh.writeFile(SD, FILE_PATH, "");

  initEncoderTimer();

  initBle();
  Serial.println("exiting setup");
}

void loop() {
  bool isBuffFullLoc = false;
  bool uploadFileLoc = false;
  //int timerExeCountLoc = 0;

  portENTER_CRITICAL_ISR(&encoderTimerMux);
  isBuffFullLoc = isBuffFull;
  uploadFileLoc = uploadFile;
  //timerExeCountLoc = timerExpCunt;
  portEXIT_CRITICAL_ISR(&encoderTimerMux);

  // Serial.println("encoderTimer exec count " + String(timerExeCountLoc));
  // Serial.println("Encoder count: " + String(Encoder::getCount()));

  if (isBuffFullLoc) {
    Serial.println("Buff is full, writing to file, ms: " + String(millis()));
    fileKnh.appendBuffToFile(SD, FILE_PATH, buff);

    portENTER_CRITICAL_ISR(&encoderTimerMux);
    isBuffFull = false;
    portEXIT_CRITICAL_ISR(&encoderTimerMux);
  }

  if (uploadFileLoc) {
    // open file
    // send one line at a time via ble
    
   File file = SD.open(FILE_PATH, FILE_READ);
    if(!file) {
      Serial.println("Failed to open file for appending");
      return;
    }

    String line = "";

    while (file.available()) {
      line = file.readStringUntil('\n');
      Serial.print(line);
      // KNH TODO send line to ble
    }
    file.close();
    SD.remove(FILE_PATH);
    
    portENTER_CRITICAL_ISR(&encoderTimerMux);
    uploadFile = false;
    portEXIT_CRITICAL_ISR(&encoderTimerMux);
  }
}