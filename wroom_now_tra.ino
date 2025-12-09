/* Transmitter: two buttons (toggle extend / stop, toggle retract / stop)
   Robust: sets pinModes for all pins, initializes last states, and only
   sends when the command actually changes (prevents continuous sends).
   RESET button always sends regardless of previous state.
   Replace receiverMAC with your receiver's MAC address (0x88,...)
*/

#include <WiFi.h>
#include <esp_now.h>

// CHANGE this to your receiver MAC (6 bytes)
uint8_t receiverMAC[] = {0x88, 0x57, 0x21, 0x79, 0x8E, 0x00};

const int buttonExtend = 27;   // Button A (toggle extend/stop)
const int buttonRetract = 26;  // Button B (toggle retract/stop)
const int b3 = 25;             // Stop pin
const int b4 = 33;             // Reset pin

// const int buttonExtend = 4;   // Button A (toggle extend/stop)
// const int buttonRetract = 3;  // Button B (toggle retract/stop)
// const int b3 = 2;             // Stop pin
// const int b4 = 1;             // Reset pin

const unsigned long debounceMs = 180;

// tx message (single byte command)
typedef struct {
  uint8_t command; // 0=STOP, 1=EXTEND, 2=RETRACT, 5=RESET
} tx_message_t;

tx_message_t txMsg;

// last button states for edge detection
int lastExtendState;
int lastRetractState;
int lastResetState;
unsigned long lastDebounceTimeExt = 0;
unsigned long lastDebounceTimeRet = 0;
unsigned long lastDebounceTimeReset = 0;
bool extendActive = false;
bool retractActive = false;

// keep last sent command so we don't resend same command repeatedly
int lastSentCommand = -1;

void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("Send: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void sendCommand(uint8_t cmd) {
  // only send if command changed
  if ((int)cmd == lastSentCommand) return;
  txMsg.command = cmd;
  esp_err_t res = esp_now_send(receiverMAC, (uint8_t*)&txMsg, sizeof(txMsg));
  if (res != ESP_OK) {
    Serial.print("esp_now_send error: ");
    Serial.println(res);
  } else {
    lastSentCommand = cmd;
    Serial.print("Sent cmd: ");
    Serial.println(cmd);
  }
}

void sendResetCommand() {
  // Force send RESET regardless of lastSentCommand
  txMsg.command = 5;
  esp_err_t res = esp_now_send(receiverMAC, (uint8_t*)&txMsg, sizeof(txMsg));
  if (res != ESP_OK) {
    Serial.print("esp_now_send error: ");
    Serial.println(res);
  } else {
    lastSentCommand = 5;
    Serial.println("TX: RESET (forced send)");
  }
}

void setup() {
  Serial.begin(9600);
  delay(100);

  // configure pins and enable internal pullups
  pinMode(buttonExtend, INPUT_PULLUP);
  pinMode(buttonRetract, INPUT_PULLUP);
  pinMode(b3, INPUT_PULLUP); // STOP pin
  pinMode(b4, INPUT_PULLUP); // RESET pin

  // initialize last states from actual readings (avoids false edges)
  lastExtendState = digitalRead(buttonExtend);
  lastRetractState = digitalRead(buttonRetract);
  lastResetState = digitalRead(b4);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true) delay(1000);
  }

  // register send callback (new signature)
  esp_now_register_send_cb(OnDataSent);

  // add peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer (maybe already added)");
  }

  // start with STOP command
  sendCommand(0);
}

void loop() {
  unsigned long now = millis();
  int curExt = digitalRead(buttonExtend);
  int curRet = digitalRead(buttonRetract);
  int stop = digitalRead(b3);
  int reset = digitalRead(b4);

  // EXTEND button (detect falling edge -> pressed)
  if (curExt == LOW && lastExtendState == HIGH && (now - lastDebounceTimeExt) > debounceMs) {
    lastDebounceTimeExt = now;
    extendActive = !extendActive;   // toggle extend state
    if (extendActive) {
      // turning extend ON, ensure retract OFF
      retractActive = false;
      sendCommand(1); // EXTEND
      Serial.println("TX: EXTEND (toggled ON)");
    } else {
      sendCommand(2); // STOP
      Serial.println("TX: STOP (from EXTEND toggle OFF)");
    }
  }

  // RETRACT button (detect falling edge -> pressed)
  if (curRet == LOW && lastRetractState == HIGH && (now - lastDebounceTimeRet) > debounceMs) {
    lastDebounceTimeRet = now;
    retractActive = !retractActive; // toggle retract
    if (retractActive) {
      extendActive = false;
      sendCommand(3); // RETRACT
      Serial.println("TX: RETRACT (toggled ON)");
    } else {
      sendCommand(4); // STOP
      Serial.println("TX: STOP (from RETRACT toggle OFF)");
    }
  }

  // RESET button (detect falling edge -> pressed)
  // Always sends RESET command regardless of previous state
  if (reset == LOW && lastResetState == HIGH && (now - lastDebounceTimeReset) > debounceMs) {
    lastDebounceTimeReset = now;
    
    // Force send RESET (bypasses duplicate check)
    sendResetCommand();
    
    // Reset both active states
    extendActive = false;
    retractActive = false;
  }

  // STOP button (active LOW) — currently disabled
  // if (stop == LOW) {
  //   sendCommand(0);
  // }

  // update last states AFTER processing (edge detection)
  lastExtendState = curExt;
  lastRetractState = curRet;
  lastResetState = reset;

  delay(20);
}




