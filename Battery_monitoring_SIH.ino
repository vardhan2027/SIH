// ----- Battery ADC Pin -----
#define BAT_ADC 34

// ----- RGB LED Pins -----
#define LED_R 14
#define LED_G 12
#define LED_B 13

// ----- Voltage Divider Values -----
const float R1 = 30000.0; // 30k
const float R2 = 10000.0; // 10k
const float DIV_RATIO = (R1 + R2) / R2;  // = 4.0

// ----- ADC Constants -----
const float ADC_MAX = 4095.0;
const float ADC_REF = 3.3;

// ----- Battery Voltage Range (12V battery) -----
const float BATTERY_FULL = 12;  // 100%
const float BATTERY_EMPTY = 0; // 0%

void setColor(int r, int g, int b) {
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  // Turn off LED initially
  setColor(0, 0, 0);
}

void loop() {

  int raw = analogRead(BAT_ADC);

  // Convert ADC raw to voltage at ADC pin
  float v_adc = (raw / ADC_MAX) * ADC_REF;

  // Convert to actual battery voltage
  float battery_voltage = v_adc * DIV_RATIO;

  // Convert voltage to %
  float percent = (battery_voltage - BATTERY_EMPTY) * 100.0 / (BATTERY_FULL);
  // if (percent > 100) percent = 100;
  // if (percent < 0) percent = 0;

  // ----- LED INDICATION -----
  if (percent > 60) {
    setColor(0, 255, 0);  // GREEN
  }
  else if (percent > 30 and percent <60) {
    setColor(255, 165, 0);  // ORANGE
  }
  else {
    // RED blinking for LOW battery
    setColor(255, 0, 0);
    delay(300);
    setColor(0, 0, 0);
    delay(300);
  }

  // Print readings
  Serial.print("Raw ADC: ");
  Serial.print(raw);
  Serial.print(" | ADC V: ");
  Serial.print(v_adc, 2);
  Serial.print(" | Battery: ");
  Serial.print(battery_voltage, 2);
  Serial.print("V | Percent: ");
  Serial.print(percent);
  Serial.println("%");

  delay(700);
}
