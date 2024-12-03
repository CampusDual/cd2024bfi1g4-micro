#include <BLEDevice.h>
#include <BLEServer.h>
#include <ArduinoJson.h>

#define SERVICE_UUID "12345678-1234-1234-1234-1234567890AB"
#define CHARACTERISTIC_UUID "12345678-1234-1234-1234-1234567890CD"

BLECharacteristic *pCharacteristic;

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        String value = pCharacteristic->getValue();
        if (!value.isEmpty()) {
            Serial.println("Configuración recibida: " + String(value.c_str()));

      
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, value);
            if (!error) {
                Serial.println("Bien al parsear JSON.");

                
            } else {
                Serial.println("Error al parsear JSON.");
            }
        }
    }
};

void setup() {
    Serial.begin(115200);

    BLEDevice::init("ESP32-C3 Config");
    BLEServer *pServer = BLEDevice::createServer();

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->start();
    Serial.println("BLE inicializado. Listo para recibir configuraciones.");
}

void loop() {
 

 NRF Connect


}