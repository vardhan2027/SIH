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
  delay(100);
}