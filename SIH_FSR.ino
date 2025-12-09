// DOIT ESP32 DevKit – 3 FSR threshold check
// Thumb -> GPIO 34
// Index -> GPIO 35
// Middle -> GPIO 32

#define THUMB_PIN   34
#define INDEX_PIN   35
#define MIDDLE_PIN  32

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

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("ESP32 – 3 FSR threshold test started...");
}

void loop() {

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
      (thumbAvg  >= SP_THUMB)  ||
      (indexAvg  >= SP_INDEX)  ||
      (middleAvg >= SP_MIDDLE);

  if (conditionOK) {
    Serial.println("RESULT: RED");
  } else {
    Serial.println("RESULT: GREEN");
  }

  Serial.println("----------------------------------");
  delay(300);  // small delay between cycles
}
