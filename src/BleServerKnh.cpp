#include "BleServerKnh.h"

//String BleServerKnh::commandReceived = "";

BleServerKnh::BleServerKnh() {
    //commandReceived = "";
}

class CallbackCommandMode: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *characteristic) {
      std::string value = characteristic->getValue();

      //BleServerKnh::commandReceived = String(value.c_str());

      if (value.length() > 0) {
        Serial.print("New command: ");
        for (int i = 0; i < value.length(); i++) {
          Serial.print(value[i]);
        }
        Serial.println("");

        Serial.println();
        Serial.println("*********");
      }
    }
};

class CallbackLastEncoderCount: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic *characteristic) {
        uint32_t m = millis();
        String mString = String(m);
        Serial.println("Ble get last encoder count callback: " + mString);
        characteristic->setValue(mString.c_str());
    }
};

void BleServerKnh::initBle() {
  BLEDevice::init("YoYo DAC");
  BLEAddress bleAddress = BLEDevice::getAddress();
  Serial.println("ble Addr: " + String(bleAddress.toString().c_str()));

  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  
  pCharacteristicCommandMode = pService->createCharacteristic(
    CHARACTERISTIC_COMMAND_MODE_UUID,
    BLECharacteristic::PROPERTY_WRITE);

  pCharacteristicCommandMode->setCallbacks(new CallbackCommandMode());
  
  pCharacteristicSample = pService->createCharacteristic(
    CHARACTERISTIC_LAST_ENCODER_COUNT_UUID,
    BLECharacteristic::PROPERTY_READ);

  pCharacteristicSample->setCallbacks( new CallbackLastEncoderCount());

  pCharacteristicSample->setValue("sample");

  pService->start();

  pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

String BleServerKnh::GetCommand() {
    std::string value = pCharacteristicCommandMode->getValue();
    return String(value.c_str());
}