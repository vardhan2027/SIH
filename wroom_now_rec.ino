/* Receiver: MX1508 motor driver (IN1/IN2) — ESP32 Core 3.x compatible
   IN1 -> GPIO26
   IN2 -> GPIO27
*/

#include <WiFi.h>
#include <esp_now.h>

const int IN1 = 26;
const int IN2 = 25;
const int IN3 = 33;
const int IN4 = 32;


#define THUMB_PIN   34   // FSR Thumb
#define INDEX_PIN   35   // FSR Index
#define MIDDLE_PIN  13   // FSR Middle

// ---- Setpoints (from your calibration) ----
const float SP_THUMB  = 825.72;
const float SP_INDEX  = 323.50;
const float SP_MIDDLE = 690.96;

// Read one FSR: ignore first 2 samples, average next 5
float readFSRavg(int pin) {
  int raw;

  // discard first 2 readings
  for (int i = 0; i < 2; i++) {
    raw = analogRead(pin);
    delay(50);
  }

  // take next 5 readings for average
  long sum = 0;
  for (int i = 0; i < 5; i++) {
    raw = analogRead(pin);
    sum += raw;
    delay(5);
  }

  float avg = sum / 5.0;
  return avg;
}

typedef struct {
  uint8_t command; // 0=stop, 1=extend, 2=retract
} rx_message_t;

rx_message_t rxMsg;

// New receive callback signature (Core 3.x)
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (data == nullptr || len < (int)sizeof(rxMsg)) {
    Serial.println("Received invalid/short packet");
    return;
  }

  // copy only the expected bytes
  memcpy(&rxMsg, data, sizeof(rxMsg));
  Serial.print("Recv cmd: ");
  Serial.println(rxMsg.command);

  switch (rxMsg.command) {
    case 0: // STOP
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);            
      Serial.println("Actuator: STOP");
      break;

    case 1: // EXTEND
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      Serial.println("Actuator: EXTEND");
      break;

    case 2: // RETRACT
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      Serial.println("Actuator: RETRACT");
      break;

    case 3:
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
      Serial.println("Both Actuator: RETRACT");
      break;

    case 4:
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
      Serial.println("Both Actuator: EXTEND");
      break;    
     
    case 5:
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN4, LOW);
      digitalWrite(IN3, HIGH);
      Serial.println("Reset");
      break;    

    // default:
    //   Serial.println("Unknown command");
    //   break;
  }
}

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  delay(100);
  Serial.println("ESP32 – 3 FSR threshold test started...");
  Serial.print("My MAC: ");
  Serial.println(WiFi.macAddress());
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // ensure stopped
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true) delay(1000);
  }

  // register receive callback (new signature)
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Receiver ready");
}

void loop() {
  // nothing here — all handled in callback
    // 1️⃣ Get averaged readings for each finger
  float thumbAvg  = readFSRavg(THUMB_PIN);
  float indexAvg  = readFSRavg(INDEX_PIN);
  float middleAvg = readFSRavg(MIDDLE_PIN);

  // 2️⃣ Print averages
  Serial.println("========== FSR AVERAGES ==========");
  Serial.print("Thumb  avg : ");  Serial.println(thumbAvg);
  Serial.print("Index  avg : ");  Serial.println(indexAvg);
  Serial.print("Middle avg : ");  Serial.println(middleAvg);

  // 3️⃣ Check AND condition for all three fingers
  bool conditionOK =
      (thumbAvg -100 >= SP_THUMB)  ||
      (indexAvg +35  >= SP_INDEX)  ||
      (middleAvg - 3000>= SP_MIDDLE);


  if(conditionOK){
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  }
  delay(100);
}