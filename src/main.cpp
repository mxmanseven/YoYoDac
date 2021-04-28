#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include "Buff.h"
#include "Encoder.h"
#include "FileKnh.h"

#include "BleServerKnh.h"

const int LED_BUILTIN_PIN = 16;
const byte encoderPinA = 27;
const byte encoderPinB = 26;

const char* FILE_PATH = "/data.txt";
File file;

// when uploading file with BLE 
// avoid writing to file and buff
bool uploadFileToBle = false;

// record samples at regular interval with a encoderTimer
hw_timer_t * encoderTimer = NULL;
portMUX_TYPE encoderTimerMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool isBuffFull = false;
volatile int timerExpCunt = 0;

Buff buff;
FileKnh fileKnh;
BleServerKnh bleServerKnh;

void IRAM_ATTR onEncoderTimer() {
  portENTER_CRITICAL_ISR(&encoderTimerMux);
  if (uploadFileToBle == true) return; // don't collect samples while downloading
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
  int interruptAtScalledTickes = 100000; //  some how this results in a timer each 100ms, with prescaler of 80.
  //int interruptAtScalledTickes = 10000; //  some how this results in a timer each 10ms, with prescaler of 80.
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

  Encoder::InitEncoder(encoderPinA, encoderPinB, LED_BUILTIN_PIN);

  fileKnh.initSdFs();
  fileKnh.writeFile(SD, FILE_PATH, "");

  initEncoderTimer();

  bleServerKnh.initBle();

  Serial.println("exiting setup");
}

void loop() {
  bool isBuffFullLoc = false;

  portENTER_CRITICAL_ISR(&encoderTimerMux);
  isBuffFullLoc = isBuffFull;
  portEXIT_CRITICAL_ISR(&encoderTimerMux);

  //Serial.println("Encoder count: " + String(Encoder::getCount()));

  if (isBuffFullLoc && uploadFileToBle == false) {
    // write buffer to file unless we are uploading file to BLE
    Serial.println("Buff is full, writing to file, ms: " + String(millis()));
    fileKnh.appendBuffToFile(SD, FILE_PATH, buff);

    portENTER_CRITICAL_ISR(&encoderTimerMux);
    isBuffFull = false;
    portEXIT_CRITICAL_ISR(&encoderTimerMux);
  }

  String bleCommand = bleServerKnh.GetCommand();
  if (bleCommand != "") {
    Serial.println("ble command from main: " + bleCommand);
  }

  if (bleCommand == "Zero") {  
    portENTER_CRITICAL_ISR(&encoderTimerMux);
    Encoder::zeroCount();
    buff.ZeroOut();
    fileKnh.deleteFile(SD, FILE_PATH);
    uploadFileToBle = false;
    portEXIT_CRITICAL_ISR(&encoderTimerMux);
  }
  else if (bleCommand == "GetLastSample") {
    String count = String(Encoder::getCount());
    Serial.println("Encoder count: " + count);
    bleServerKnh.pCharacteristicSample->setValue(count.c_str());
  }
  else if (bleCommand == "Download") {
    if (!file) {      
      portENTER_CRITICAL_ISR(&encoderTimerMux);
      uploadFileToBle = true;
      portEXIT_CRITICAL_ISR(&encoderTimerMux);
      file = SD.open(FILE_PATH, FILE_READ);
        if(!file) {
          Serial.println("Failed to open file for appending");
        }
    }

    if (file.available()) {
      String line = "";
      line = file.readStringUntil('\n');
      Serial.print(line);
      bleServerKnh.pCharacteristicSample->setValue(line.c_str());
    }
    else {
      bleServerKnh.pCharacteristicSample->setValue("");
      file.close();
      SD.remove(FILE_PATH);
      buff.ZeroOut();
      
      portENTER_CRITICAL_ISR(&encoderTimerMux);
      uploadFileToBle = false;
      portEXIT_CRITICAL_ISR(&encoderTimerMux);
    }
  }
}