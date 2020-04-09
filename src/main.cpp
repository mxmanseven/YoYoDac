#include <Arduino.h>
#include "BluetoothSerial.h"
#include "Buff.h"
#include "Encoder.h"
#include "FileKnh.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// // Libraries for SD card
// #include "FS.h"
// #include "SD.h"
// #include <SPI.h>

// Define CS pin for the SD card module
#define SD_CS_PIN 5

const int LED_BUILTIN_PIN = 2;
const byte encoderPinA = 27;
const byte encoderPinB = 26;

volatile uint32_t count = 0;
uint32_t protectedCount = 0;
uint32_t previousCount = 0;

String dataMessage;

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

int readingID = 1000;

// Write the sensor readings on the SD card
void logSDCard() {
  dataMessage = String(readingID++) + "\r\n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  fileKnh.appendFile(SD, "/data.txt", dataMessage.c_str());
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode (LED_BUILTIN_PIN, OUTPUT);
  initEncoder();
  //initBlueTooth();
  //initSdFs();

  //buff.Test();

  fileKnh.initSdFs(SD_CS_PIN);
  logSDCard();
}

void loop() {  
  // if (Serial.available()) {
  //   SerialBT.write(Serial.read());
  // }
  // if (SerialBT.available()) {
  //   Serial.write(SerialBT.read());
  // }
  // delay(2000);

  // logSDCard();
}