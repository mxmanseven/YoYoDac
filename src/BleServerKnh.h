#ifndef BLESERVERKNH_H
#define BLESERVERKNH_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <Encoder.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID                                 "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_COMMAND_MODE_UUID             "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_LAST_ENCODER_COUNT_UUID       "cda2ac87-84a9-4cbc-8de8-f0285889a4e9"
#define CHARACTERISTIC_NEXT_LINE_OF_DATA_UUID        "05435f99-ce01-4905-a845-e863cebf3654"

class BleServerKnh {
    public:
    BleServerKnh();
    void initBle();
    String GetCommand();

    BLEServer* pServer;
    BLEService* pService;
    BLEAdvertising* pAdvertising;
    BLECharacteristic* pCharacteristicCommandMode;
    BLECharacteristic* pCharacteristicSample;
    BLECharacteristic* pCharacteristicNextLineOfData;

    BLECharacteristicCallbacks* callbackNextLineOfDate;

    //Encoder* encoder;
};

#endif