// ================= PIN SETUP =================
#define B1 13   // Button 1 - LA1 only
#define B2 12   // Button 2 - Both actuators
#define B3 25   // Button 3 - Reset (open both)

#define LA1_IN1 0
#define LA1_IN2 4
#define LA2_IN3 26
#define LA2_IN4 27

// ============ STATE VARIABLES ============
bool la1ForwardState = true;       // direction toggle for LA1
bool laBothForwardState = true;    // direction toggle for both

unsigned long debounce = 300;
unsigned long lastPress = 0;

// // Travel time based on 50mm stroke @ 15mm/s = 3.33s → use 3.5s
// #define FULL_TRAVEL_TIME 3500


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

  digitalWrite(LA1_IN1, HIGH);
  digitalWrite(LA1_IN2, LOW);
  digitalWrite(LA2_IN3, HIGH);
  digitalWrite(LA2_IN4, LOW);
  // delay(FULL_TRAVEL_TIME);

  // stopAll();
  Serial.println("Reset complete.");
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
  if (!digitalRead(B1) && millis() - lastPress > debounce) {
    lastPress = millis();

    if (la1ForwardState) {
      Serial.println("LA1 FORWARD");
      la1Forward();
      //delay(FULL_TRAVEL_TIME);
      //stopAll();
    } else {
      Serial.println("LA1 BACKWARD");
      la1Backward();
      //delay(FULL_TRAVEL_TIME);
      //stopAll();
    }

    la1ForwardState = !la1ForwardState;
  }


  // -------- Button 2: Control both actuators ---------
  if (!digitalRead(B2) && millis() - lastPress > debounce) {
    lastPress = millis();

    resetBoth();  // must open fully before any action

    if (laBothForwardState) {
      Serial.println("BOTH FORWARD");
      la1Forward();
      la2Forward();
      // delay(FULL_TRAVEL_TIME);
      // stopAll();
    } else {
      Serial.println("BOTH BACKWARD");
      la1Backward();
      la2Backward();
      // delay(FULL_TRAVEL_TIME);
      // stopAll();
    }

    laBothForwardState = !laBothForwardState;
  }


  // -------- Button 3: Hard Reset both ---------
  if (!digitalRead(B3) && millis() - lastPress > debounce) {
    lastPress = millis();
    resetBoth();
    
  }
}