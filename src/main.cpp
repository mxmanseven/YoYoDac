#include <Arduino.h>
#include "BluetoothSerial.h"
#include "Buff.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Libraries for SD card
#include "FS.h"
#include "SD.h"
#include <SPI.h>

// Define CS pin for the SD card module
#define SD_CS_PIN 5

const int LED_BUILTIN_PIN = 2;
const byte encoderPinA = 27;
const byte encoderPinB = 26;

volatile uint32_t count = 0;
uint32_t protectedCount = 0;
uint32_t previousCount = 0;

int ledState = HIGH;

String dataMessage;

BluetoothSerial SerialBT;

Buff buff;

// rotory a chanel
void IRAM_ATTR isrA() {
  if(digitalRead(encoderPinA) != digitalRead(encoderPinB)) {
    count ++;
    digitalWrite(LED_BUILTIN_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else {
    count --;
    digitalWrite(LED_BUILTIN_PIN, LOW);   // turn the LED on (HIGH is the voltage level)
  }
}

// rotory b chanel
void IRAM_ATTR  isrB() {
  if (digitalRead(encoderPinA) == digitalRead(encoderPinB)) {
    count ++;
    digitalWrite(LED_BUILTIN_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else {
    count --;
    digitalWrite(LED_BUILTIN_PIN, LOW);   // turn the LED on (HIGH is the voltage level)
  }
}

void EncoderSetup() {
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(encoderPinA), isrA, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(encoderPinB), isrB, CHANGE);
  // dissable one of the interrupts to reduce the resolution by half
  // with the amazon ldp3806-600bn this leads to about 1200 ticks per rotation - more than enough
}

void initBlueTooth() {  
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth! knh");
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void initSdFs() {
  // Initialize SD card
  SD.begin(SD_CS_PIN);  
  if(!SD.begin(SD_CS_PIN)) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }

  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/data.txt");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.txt", "Reading ID, Date, Hour, Temperature \r\n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode (LED_BUILTIN_PIN, OUTPUT);
  //EncoderSetup();
  //initBlueTooth();
  //initSdFs();

  buff.Test();
}

int readingID = 0;

// Write the sensor readings on the SD card
void logSDCard() {
  dataMessage = String(readingID++) + "\r\n";
  Serial.print("Save data: ");
  Serial.println(dataMessage);
  appendFile(SD, "/data.txt", dataMessage.c_str());
}

void loop() {  
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
  delay(2000);

  logSDCard();
}