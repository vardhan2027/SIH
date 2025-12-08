// ================= PIN SETUP =================
#define B1 13   // Button 1 - LA1 only
#define B2 12   // Button 2 - Both actuators
#define B3 25   // Button 3 - Reset (open both)

#define LA1_IN1 0
#define LA1_IN2 4
#define LA2_IN3 26
#define LA2_IN4 27

// ============ STATE VARIABLES ============
bool la1ForwardState = true;       
bool laBothForwardState = true;    

unsigned long debounce = 300;
unsigned long lastPressB1 = 0;
unsigned long lastPressB2 = 0;
unsigned long lastPressB3 = 0;


// ================= MOTOR CONTROL =================

void la1Forward() {
  digitalWrite(LA1_IN1, HIGH);
  digitalWrite(LA1_IN2, LOW);
}

void la1Backward() {
  digitalWrite(LA1_IN1, LOW);
  digitalWrite(LA1_IN2, HIGH);
}

void la2Forward() {
  digitalWrite(LA2_IN3, HIGH);
  digitalWrite(LA2_IN4, LOW);
}

void la2Backward() {
  digitalWrite(LA2_IN3, LOW);
  digitalWrite(LA2_IN4, HIGH);
}

void stopAll() {
  digitalWrite(LA1_IN1, LOW);
  digitalWrite(LA1_IN2, LOW);
  digitalWrite(LA2_IN3, LOW);
  digitalWrite(LA2_IN4, LOW);
}


// ============ RESET FUNCTION (Fully Open) =============
void resetBoth() {
  Serial.println("Reset: Opening both actuators fully...");

  // Open both
  la1Forward();
  la2Forward();
  // delay(FULL_TRAVEL_TIME);

  // stopAll();

  // After reset → next action must be backward
  la1ForwardState = false;
  laBothForwardState = false;

  Serial.println("Reset complete. Next action = BACKWARD.");
}



// ================= SETUP ==================
void setup() {
  Serial.begin(115200);

  pinMode(B1, INPUT_PULLUP);
  pinMode(B2, INPUT_PULLUP);
  pinMode(B3, INPUT_PULLUP); 

  pinMode(LA1_IN1, OUTPUT);
  pinMode(LA1_IN2, OUTPUT);
  pinMode(LA2_IN3, OUTPUT);
  pinMode(LA2_IN4, OUTPUT); 

  stopAll();
  Serial.println("System Ready");
}



// ================= MAIN LOOP ==================
void loop() {

  // -------- Button 1: Control LA1 only ---------
  if (!digitalRead(B1) && millis() - lastPressB1 > debounce) {
    lastPressB1 = millis();

    if (la1ForwardState) {
      Serial.println("LA1 FORWARD");
      la1Forward();
    } else {
      Serial.println("LA1 BACKWARD");
      la1Backward();
    }

    la1ForwardState = !la1ForwardState;
  }


  // -------- Button 2: Toggle both actuators ---------
  if (!digitalRead(B2) && millis() - lastPressB2 > debounce) {
    lastPressB2 = millis();

    if (laBothForwardState) {
      Serial.println("BOTH FORWARD");
      la1Forward();
      la2Forward();
    } else {
      Serial.println("BOTH BACKWARD");
      la1Backward();
      la2Backward();
    }

    laBothForwardState = !laBothForwardState;
  }


  // -------- Button 3: Hard Reset (open both) ---------
  if (!digitalRead(B3) && millis() - lastPressB3 > debounce) {
    lastPressB3 = millis();
    resetBoth();
  }

}