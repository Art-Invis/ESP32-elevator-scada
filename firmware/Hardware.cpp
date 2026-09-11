#include "Hardware.h"
#include "Config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Локальні змінні для заліза
float R0 = 76.63;
bool lastFanState = false;
unsigned long lastFanOn = 0;

int currentScreen = 0;
bool screenChanged = true;
unsigned long lastAlarmDisplay = 0;

bool lastButtonState = HIGH;
bool lastFanButtonState = HIGH;
unsigned long lastButtonPress = 0;
unsigned long lastFanButtonPress = 0;

void initializeHardware() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  pinMode(MOTION_SENSOR_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(FAN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(WHITE_LED_PIN, OUTPUT);
  digitalWrite(WHITE_LED_PIN, LOW);
  pinMode(FAN_LED_PIN, OUTPUT);
  digitalWrite(FAN_LED_PIN, LOW);
  
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);
  setRGBColor(0, 0, 0);

  initializeOLED();
  startupSequence();
}

void initializeOLED() {
  Wire.begin(SDA_PIN, SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("❌ OLED init failed");
    setRGBColor(255, 0, 0);
    while(1);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void startupSequence() {
  setRGBColor(128, 0, 128); delay(500);
  setRGBColor(0, 128, 128); delay(500);
  setRGBColor(0, 0, 255); delay(500);
  setRGBColor(0, 255, 0); delay(500);
}

void setRGBColor(int red, int green, int blue) {
  analogWrite(RGB_RED, red);
  analogWrite(RGB_GREEN, green);
  analogWrite(RGB_BLUE, blue);
}

void updateFanLED() {
  digitalWrite(FAN_LED_PIN, elevator.fan_running ? HIGH : LOW);
}

void updateSystemIndicator() {
  if (!elevator.system_enabled) {
    setRGBColor(128, 0, 128);
    return;
  }
  switch(elevator.air_quality) {
    case EXCELLENT: setRGBColor(0, 255, 0); break;
    case GOOD: setRGBColor(255, 255, 0); break;
    case MODERATE: setRGBColor(255, 165, 0); break;
    case POOR: setRGBColor(255, 50, 0); break;
    case DANGEROUS:
      setRGBColor((millis() % 1000 < 500) ? 255 : 0, 0, 0);
      break;
  }
}

void controlFan(bool state) {
  if (!elevator.system_enabled && state) return;
  
  digitalWrite(RELAY_PIN, !state);
  elevator.fan_running = state;
  updateFanLED();
  
  if(state && !lastFanState) {
    fanCycles++;
    lastFanOn = millis();
    Serial.println("💨 Fan ON");
  } else if(!state && lastFanState) {
    unsigned long onDuration = millis() - lastFanOn;
    fanOnTime += onDuration;
    Serial.println("🔇 Fan OFF");
  }
  lastFanState = state;
}

void controlLight(bool state) {
  if (!elevator.system_enabled && state) return;
  elevator.light_running = state;
  digitalWrite(WHITE_LED_PIN, state ? HIGH : LOW);
  
  if (state) {
    elevator.light_timeout = millis() + LIGHT_DURATION;
    Serial.println("💡 Light ON");
  } else {
    Serial.println("🌙 Light OFF");
  }
}

void handleButtons() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    if (millis() - lastButtonPress > 300) {
      elevator.system_enabled = !elevator.system_enabled;
      lastButtonPress = millis();
      Serial.print("⚡ System ");
      Serial.println(elevator.system_enabled ? "🟢 ENABLED" : "🔴 DISABLED");
      if (!elevator.system_enabled) {
        controlFan(false);
        controlLight(false);
      }
    }
  }
  lastButtonState = currentButtonState;
  
  bool currentFanButtonState = digitalRead(FAN_BUTTON_PIN);
  if (currentFanButtonState == LOW && lastFanButtonState == HIGH) {
    if (millis() - lastFanButtonPress > 300) {
      if (elevator.system_enabled) {
        controlFan(!elevator.fan_running);
        Serial.print("🎮 Fan ");
        Serial.println(elevator.fan_running ? "🟢 MANUAL ON" : "🔴 MANUAL OFF");
      }
      lastFanButtonPress = millis();
    }
  }
  lastFanButtonState = currentFanButtonState;
}

void calibrateMQ135() {
  setRGBColor(255, 255, 0);
  Serial.println("🔧 Calibrating MQ135...");
  delay(5000); // Скоротив для тестування
  float sum = 0;
  for(int i = 0; i < 50; i++) {
    sum += getResistance();
    delay(50);
  }
  R0 = sum / 50;
  Serial.print("🎯 R0 calibrated: ");
  Serial.println(R0);
  setRGBColor(0, 255, 0);
}

float getResistance() {
  int raw = analogRead(MQ135_PIN);
  float voltage = raw * (3.3 / 4095.0);
  return (3.3 - voltage) / voltage * 10000.0;
}

float getPPM() {
  float rs = getResistance();
  float ratio = rs / R0;
  return 116.6020682 * pow(ratio, -2.769034857);
}

String getQualityString(AirQualityLevel q) {
  switch(q) {
    case EXCELLENT: return "EXCELLENT";
    case GOOD: return "GOOD";
    case MODERATE: return "MODERATE";
    case POOR: return "POOR";
    case DANGEROUS: return "DANGEROUS";
  }
  return "UNKNOWN";
}

void updateDisplay() {
  display.clearDisplay();
  
  if (elevator.air_quality == DANGEROUS && millis() - lastAlarmDisplay < 5000) {
    display.setTextSize(2);
    display.setCursor(10, 0);
    display.println("! ALARM !");
    display.setTextSize(1);
    display.setCursor(0, 25);
    display.println("CRITICAL AIR QUALITY");
  } else {
    elevator.alarm_displayed = false;
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("STATUS | Scr: ");
    display.print(currentScreen);
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    display.setCursor(0, 15);
    display.print("CO2: ");
    display.print((int)elevator.co2_ppm);
    display.print(" ppm");
    display.setCursor(0, 30);
    display.print("Fan: ");
    display.print(elevator.fan_running ? "ON " : "OFF");
  }
  display.display();
}

void switchScreen() {
  currentScreen = (currentScreen + 1) % 3;
  screenChanged = true;
}

void printSystemStatus() {
  Serial.println("\n--- SYSTEM STATUS ---");
  Serial.print("CO2: "); Serial.print(elevator.co2_ppm, 0); Serial.println(" ppm");
  Serial.print("Fan: "); Serial.println(elevator.fan_running ? "ON" : "OFF");
  Serial.print("Light: "); Serial.println(elevator.light_running ? "ON" : "OFF");
  Serial.println("---------------------\n");
}
