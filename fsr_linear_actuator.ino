/* ESP32 MX1508 + BLE + 2 FSR Auto-Stop
   - Supports MODE3 (single actuator) and MODE5 (both actuators)
   - BLE command characteristic accepts: MODE3, MODE5, OPEN, CLOSE, STOP
   - Battery service (read + notify) kept as before
   - Auto-stop: if either FSR reads inside configured range, motors stop
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// ------------------------
// Motor Pins
// ------------------------
const int IN1 = 26;
const int IN2 = 25;
const int IN3 = 33;
const int IN4 = 32;

// ------------------------
// FSR Pins (ADC)
// ------------------------
#define THUMB_PIN 13
#define INDEX_PIN 34

// ------------------------
// Thresholds (adjust to your sensors / calibration)
// ------------------------
#define THUMB_MIN 2200
#define THUMB_MAX 4500

#define INDEX_MIN 1120
#define INDEX_MAX 1237.916

// ------------------------
// Read FSR (simple average filter)
// ------------------------
float readFSR(int pin) {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(pin);
    delay(3);
  }
  return sum / 10.0;
}

// ------------------------
// BLE UUIDs
// ------------------------
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define WRITE_UUID          "12345678-1234-1234-1234-1234567890ac"

// Standard battery service
#define BATTERY_SERVICE_UUID BLEUUID((uint16_t)0x180F)
#define BATTERY_LEVEL_UUID   BLEUUID((uint16_t)0x2A19)

// ------------------------
// BLE Objects & Globals
// ------------------------
BLEServer*         server;
BLECharacteristic* writeChar;
BLECharacteristic* batteryChar;

bool    deviceConnected = false;
uint8_t batteryLevel    = 90;

// Current mode selected via BLE: "MODE3" or "MODE5"
String currentMode = "MODE3";   // default mode

// -------------------------------------------------
// SERVER CALLBACKS
// -------------------------------------------------
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* s) override {
    deviceConnected = true;
    Serial.println("🔵 Device Connected");
  }

  void onDisconnect(BLEServer* s) override {
    deviceConnected = false;
    Serial.println("🔴 Device Disconnected");

    delay(200);
    BLEDevice::startAdvertising();  // Restart advertising safely
    Serial.println("📡 Advertising Restarted");
  }
};

// -------------------------------------------------
// WRITE CALLBACKS  (ALL COMMAND HANDLING HERE)
// -------------------------------------------------
class WriteCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* ch) override {

    // Use Arduino String because many BLE libraries return Arduino String
    String cmd = ch->getValue();

    if (cmd.length() == 0) return;

    cmd.trim();  // remove any \n, \r, spaces

    Serial.print("📩 Received Command: ");
    Serial.println(cmd);

    // ---------- MODE SELECT ----------
    if (cmd == "MODE3" || cmd == "MODE5") {
      currentMode = cmd;
      Serial.print("✅ Mode changed to: ");
      Serial.println(currentMode);
      return;   // only mode change, no motor action
    }

    // ---------- ACTIONS ----------
    if (cmd == "OPEN") {
      if (currentMode == "MODE3") {
        // Single actuator RETRACT (IN1/IN2 control)
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        Serial.println("Actuator: RETRACT (MODE3 OPEN)");
      }
      else if (currentMode == "MODE5") {
        // Both actuators RETRACT
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        Serial.println("Both Actuators: RETRACT (MODE5 OPEN)");
      }
    }
    else if (cmd == "CLOSE") {
      if (currentMode == "MODE3") {
        // Single actuator EXTEND
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        Serial.println("Actuator: EXTEND (MODE3 CLOSE)");
      }
      else if (currentMode == "MODE5") {
        // Both actuators EXTEND
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        Serial.println("Both Actuators: EXTEND (MODE5 CLOSE)");
      }
    }
    else if (cmd == "STOP") {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      Serial.println("Actuators: STOP");
    }
    else {
      Serial.println("⚠ Unknown command");
    }
  }
};

// -------------------------------------------------
// SETUP
// -------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);

  // Motor Pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Ensure all motors OFF
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  // FSR Pins - ADC pins don't strictly require pinMode(INPUT) but it's harmless
  pinMode(THUMB_PIN, INPUT);
  pinMode(INDEX_PIN, INPUT);

  // ---- BLE INIT ----
  Serial.println("📡 Starting BLE...");
  BLEDevice::init("DEXTRA-ESP32");   // Device Name

  server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  // ---- Control Service (for commands) ----
  BLEService* controlService = server->createService(SERVICE_UUID);

  writeChar = controlService->createCharacteristic(
                WRITE_UUID,
                BLECharacteristic::PROPERTY_WRITE
              );
  writeChar->setCallbacks(new WriteCallbacks());

  controlService->start();

  // ---- Battery Service (optional) ----
  BLEService* batteryService = server->createService(BATTERY_SERVICE_UUID);

  batteryChar = batteryService->createCharacteristic(
                  BATTERY_LEVEL_UUID,
                  BLECharacteristic::PROPERTY_READ |
                  BLECharacteristic::PROPERTY_NOTIFY
                );
  batteryChar->setValue(&batteryLevel, 1);
  batteryService->start();

  // ---- Advertising ----
  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->addServiceUUID(BATTERY_SERVICE_UUID);

  // improve compatibility with Android devices
  adv->setScanResponse(false);
  adv->setMinPreferred(0x06);
  adv->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  Serial.println("✅ BLE Ready. Advertising...");
}

// -------------------------------------------------
// LOOP → AUTO-STOP USING FSR
// -------------------------------------------------
void loop() {

  float thumb = readFSR(THUMB_PIN);
  float index = readFSR(INDEX_PIN);

  Serial.print("Thumb: ");
  Serial.println(thumb);
  Serial.print("Index: ");
  Serial.println(index);

  bool thumbOK = (thumb >= THUMB_MIN && thumb <= THUMB_MAX);
  bool indexOK = (index >= INDEX_MIN && index <= INDEX_MAX);

  if (thumbOK || indexOK) {
    // stop all motors if any FSR triggers auto-stop
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);

    Serial.println("🛑 AUTO STOP (FSR TRIGGER)");
  }

  // (Optional) update battery characteristic if needed
  // batteryChar->setValue(&batteryLevel,1);
  // batteryChar->notify();

  delay(100);
}