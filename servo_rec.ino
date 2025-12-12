#include <ESP32Servo.h>
#include <WiFi.h>
#include <esp_now.h>

Servo servo1; 
Servo servo2;

#define GRAB_SERVO 18
#define PICK_SERVO 19
#define PICK 13   // Clockwise button
#define RELEASE  12   // Anti-clockwise button
#define STOP 14

int pos = 0;  // Start at mid position

typedef struct {
  uint8_t command; // 0=stop, 1=extend, 2=retract
} rx_message_t;

  rx_message_t rxMsg;


void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (data == nullptr || len < (int)sizeof(rxMsg)) {
    Serial.println("Received invalid/short packet");
    return;
  }
  // copy only the expected bytes 
  memcpy(&rxMsg, data, sizeof(rxMsg));
  Serial.print("Recv cmd: ");
  Serial.println(rxMsg.command);

  switch(rxMsg.command){
    case 0: //Stop
      servo1.write(90);
      servo2.write(90);
      delay(1500);      
      break;
    case 1: // 3 Finger open
      servo1.write(0);
      servo2.write(90);
      delay(1500);
      break;
    case 2: // 3 Finger close
      servo1.write(180);
      servo2.write(90);
      delay(1500);      
      break;  
    case 3: // 5 Finger Close
      servo1.write(0);
      servo2.write(0);
      // delay(2000);      
      break;
    case 4: // 5 Finger Open
      servo1.write(180);
      servo2.write(180);
      break;
    case 5:
      servo1.write(180);
      servo2.write(180);
      break;  
    }
}


void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  delay(100);
  Serial.println("ESP32 – 3 FSR threshold test started...");
  Serial.print("My MAC: ");
  Serial.println(WiFi.macAddress());
  servo1.attach(PICK_SERVO);
  servo2.attach(GRAB_SERVO);
  pinMode(PICK, INPUT_PULLDOWN);
  pinMode(RELEASE, INPUT_PULLDOWN);
  pinMode(STOP,INPUT_PULLDOWN);
  servo1.write(90);
  servo2.write(90);

  if (esp_now_init() != ESP_OK) {
  Serial.println("ESP-NOW init failed");
  while (true) delay(1000);
  }

  // register receive callback (new signature)
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Receiver ready");

}

void loop() {

}
