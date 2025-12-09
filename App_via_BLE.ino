#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// ------------------------
// UUIDs
// ------------------------
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define WRITE_UUID          "12345678-1234-1234-1234-1234567890ac"

// Standard battery service
#define BATTERY_SERVICE_UUID BLEUUID((uint16_t)0x180F)
#define BATTERY_LEVEL_UUID   BLEUUID((uint16_t)0x2A19)

// ------------------------
// GLOBALS
// ------------------------
BLEServer* server;
BLECharacteristic* writeChar;
BLECharacteristic* batteryChar;

bool deviceConnected = false;
uint8_t batteryLevel = 90;

// ------------------------
// SERVER CALLBACKS
// ------------------------
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* s) {
    deviceConnected = true;
    Serial.println("🔵 Device Connected");
  }

  void onDisconnect(BLEServer* s) {
    deviceConnected = false;
    Serial.println("🔴 Device Disconnected");

    delay(200);
    BLEDevice::startAdvertising();  // Restart advertising safely
    Serial.println("📡 Advertising Restarted");
  }
};

// ------------------------
// WRITE CALLBACKS
// ------------------------
class WriteCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* ch) {

    String cmd = ch->getValue().c_str();  // ALWAYS SAFE

    if (cmd.length() == 0) return;

    Serial.print("📩 Received Command: ");
    Serial.println(cmd);

    // Command handling
    if (cmd == "MODE3") Serial.println("➡ 3-Finger Mode Triggered");
    else if (cmd == "MODE5") Serial.println("➡ 5-Finger Mode Triggered");
    else if (cmd == "OPEN") Serial.println("➡ Open Hand Triggered");
    else if (cmd == "CLOSE") Serial.println("➡ Close Hand Triggered");
    else Serial.println("⚠ Unknown Command");
  }
};


// ------------------------
// SETUP
// ------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE...");

  BLEDevice::init("DEXTRA-ESP32");

  server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* controlService = server->createService(SERVICE_UUID);
  writeChar = controlService->createCharacteristic(
                WRITE_UUID,
                BLECharacteristic::PROPERTY_WRITE
              );
  writeChar->setCallbacks(new WriteCallbacks());
  controlService->start();

  BLEService* batteryService = server->createService(BATTERY_SERVICE_UUID);
  batteryChar = batteryService->createCharacteristic(
                  BATTERY_LEVEL_UUID,
                  BLECharacteristic::PROPERTY_READ |
                  BLECharacteristic::PROPERTY_NOTIFY
                );
  batteryChar->setValue(&batteryLevel, 1);
  batteryService->start();

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->addServiceUUID(BATTERY_SERVICE_UUID);
  adv->setScanResponse(true);

  BLEDevice::startAdvertising();

  Serial.println("BLE Advertising Started ✔");
}

// ------------------------
// LOOP - BATTERY SIMULATION
// ------------------------
void loop() {
  if (deviceConnected) {
    if (batteryLevel > 15) {
      batteryLevel--;
      batteryChar->setValue(&batteryLevel, 1);
      batteryChar->notify();

      Serial.print("🔋 Battery Level: ");
      Serial.println(batteryLevel);
    }

    delay(5000); // Simulate drain every 5 seconds
  }
}