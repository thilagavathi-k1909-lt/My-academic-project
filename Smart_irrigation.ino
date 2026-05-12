#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Pin definitions
#define SOIL_SENSOR 34   // Soil sensor analog pin
#define RELAY_PIN   26   // Relay pin
#define BUZZER_PIN  27   // Buzzer pin
#define LIGHT_PIN   2    // LED light pin
#define BUTTON_PIN  25   // Push button pin
// Global variables
int soilValue = 0;
int moisturePercent = 0;
int threshold = 35;   // default
int cropMode = 0;     // crop selection index
// Crop names + thresholds
const char* crops[] = {"Tomato", "Peanut", "Cotton", "Paddy"};
int cropThresholds[] = {40, 30, 35, 25}; // each crop threshold
// Function prototypes
void displayIntro();
void displaySoilStatus(int percent);
void controlPump(int percent);
void controlLight(int percent);
void beepBuzzer(int times);
void logSerialData(int rawValue, int percent);
void systemCheck();
void setThreshold(int mode);
void setup() {
  Serial.begin(115200);
  // Pin config
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(RELAY_PIN, HIGH); // Relay OFF (LOW trigger)
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LIGHT_PIN, LOW);
  // OLED init
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 not found"));
    for(;;); // Stop if no display
  }
  // Welcome screen
  displayIntro();
  delay(2000);
  // System check
  systemCheck();
  delay(2000);
  // Default crop mode
  setThreshold(cropMode);
void loop() {
  // --- Button handling with debounce ---
  static int lastButtonState = HIGH;
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    delay(20); // debounce delay
    if (reading == LOW) {   // button pressed
      cropMode++;
      if (cropMode > 3) cropMode = 0;
      setThreshold(cropMode);
  lastButtonState = reading;
  // Read soil moisture
  soilValue = analogRead(SOIL_SENSOR);
  moisturePercent = map(soilValue, 0, 4095, 100, 0);
  // Display soil + crop info
  displaySoilStatus(moisturePercent);
  // Log to Serial
  logSerialData(soilValue, moisturePercent);
  // Control devices
  controlPump(moisturePercent);
  controlLight(moisturePercent);
  delay(2000);
// ----------- FUNCTIONS ------------
// OLED intro screen
void displayIntro() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("SMART");
  display.setCursor(10, 35);
  display.println("IRRIGATION");
  display.display();
// Display soil status
void displaySoilStatus(int percent) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Crop: ");
  display.println(crops[cropMode]);
  display.setCursor(0, 12);
  display.print("Threshold: ");
  display.print(threshold);
  display.println("%");
  display.setTextSize(2);
  display.setCursor(10, 28);
  display.print(percent);
  display.println("%");
  display.setTextSize(1);
  display.setCursor(0, 55);
  if (percent < threshold) {
    display.println("Soil DRY -> Pump ON");
  } else {
    display.println("Soil WET -> Pump OFF");
  }
  display.display();
}
// Control pump and buzzer
void controlPump(int percent) {
  if (percent < threshold) {
    digitalWrite(RELAY_PIN, LOW);   // Pump ON
    digitalWrite(BUZZER_PIN, HIGH); // Buzzer ON
    beepBuzzer(1);
  } else {
    digitalWrite(RELAY_PIN, HIGH);  // Pump OFF
    digitalWrite(BUZZER_PIN, LOW);  // Buzzer OFF
  }
}
// Control light (flash when soil dry)
void controlLight(int percent) {
  if (percent < threshold) {
    for (int i = 0; i < 2; i++) {
      digitalWrite(LIGHT_PIN, HIGH);
      delay(200);
      digitalWrite(LIGHT_PIN, LOW);
      delay(200);
    }
  } else {
    digitalWrite(LIGHT_PIN, LOW); 
  }
}
// Buzzer alert
void beepBuzzer(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}
// Serial Monitor log
void logSerialData(int rawValue, int percent) {
  Serial.println("---- Soil Data Log ----");
  Serial.print("Crop: ");
  Serial.println(crops[cropMode]);
  Serial.print("Raw Value: ");
  Serial.println(rawValue);
  Serial.print("Moisture: ");
  Serial.print(percent);
  Serial.println("%");
  Serial.print("Threshold: ");
  Serial.println(threshold);
  Serial.println("------------------------");
}
// System check
void systemCheck() {
  Serial.println("Running system diagnostics...");
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("System Diagnostics...");
  display.display();
  // Relay test
  display.setCursor(0, 15);
  display.println("Testing Relay...");
  display.display();
  digitalWrite(RELAY_PIN, LOW);
  delay(500);
  digitalWrite(RELAY_PIN, HIGH);
  // Buzzer test
  display.setCursor(0, 25);
  display.println("Testing Buzzer...");
  display.display();
  beepBuzzer(2);
  // Light test
  display.setCursor(0, 35);
  display.println("Testing Light...");
  display.display();
  digitalWrite(LIGHT_PIN, HIGH);
  delay(300);
  digitalWrite(LIGHT_PIN, LOW);
  // Sensor test
  int testValue = analogRead(SOIL_SENSOR);
  Serial.print("Initial sensor value: ");
  Serial.println(testValue);
  display.setCursor(0, 50);
  display.println("All OK!");
  display.display();
  delay(1000);
}
// Set threshold for crops
void setThreshold(int mode) {
  threshold = cropThresholds[mode];
  Serial.print("Selected Crop: ");
  Serial.println(crops[mode]);
  Serial.print("Threshold set to: ");
  Serial.println(threshold);
}

