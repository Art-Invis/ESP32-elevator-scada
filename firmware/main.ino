#include "Config.h"
#include "SystemState.h"
#include "Hardware.h"
#include "Network.h"

// Ініціалізація глобальних змінних
ElevatorState elevator = {
  .co2_ppm = 0, .air_quality = EXCELLENT, .fan_running = false, .motion_detected = false, 
  .light_running = false, .light_timeout = 0, .system_enabled = true, .auto_mode = true, 
  .threshold = 300, .alarm_displayed = false
};

unsigned long motionCount = 0;
int fanCycles = 0;
unsigned long fanOnTime = 0;

// Таймери головного циклу
unsigned long lastSensorUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastFastPublish = 0;
unsigned long lastSlowPublish = 0;
unsigned long lastScreenSwitch = 0;
bool lastMotionState = false;

AirQualityLevel calculateAirQualityLevel(float ppm) {
  if (ppm < 600) return EXCELLENT;
  if (ppm < 800) return GOOD;
  if (ppm < 1000) return MODERATE;
  if (ppm < 1500) return POOR;
  return DANGEROUS;
}

void updateAirQualityLogic() {
  elevator.co2_ppm = getPPM();
  AirQualityLevel new_quality = calculateAirQualityLevel(elevator.co2_ppm);
  
  if (new_quality == DANGEROUS && !elevator.alarm_displayed) {
    elevator.alarm_displayed = true;
    Serial.println("🚨 CRITICAL AIR QUALITY!");
  }
  
  elevator.air_quality = new_quality;
  
  if (elevator.auto_mode && elevator.system_enabled) {
    bool should_fan_run = (elevator.air_quality >= MODERATE);
    
    if (should_fan_run && !elevator.fan_running) {
      controlFan(true);
    } else if (!should_fan_run && elevator.fan_running && elevator.air_quality <= GOOD) {
      controlFan(false);
    }
  }
}

void updateLightingLogic() {
  bool current_motion = digitalRead(MOTION_SENSOR_PIN);
  
  if (current_motion && !lastMotionState) {
      motionCount++;
      publishMotionEvent(true);
  }
  lastMotionState = current_motion;

  if (current_motion && !elevator.motion_detected && elevator.system_enabled) {
    elevator.motion_detected = true;
    elevator.light_timeout = millis() + LIGHT_DURATION;
    
    if (!elevator.light_running) {
      controlLight(true);
    }
  }
  
  elevator.motion_detected = current_motion;
  
  if (elevator.light_running && millis() > elevator.light_timeout) {
    controlLight(false);
  }
  
  if (elevator.air_quality >= POOR && !elevator.light_running && elevator.system_enabled) {
    controlLight(true);
  }
}

void handleSerialCommand(char cmd) {
  switch(cmd) {
    case 'a': elevator.auto_mode = true; Serial.println("🤖 AUTO mode"); break;
    case 'm': elevator.auto_mode = false; Serial.println("🎮 MANUAL mode"); break;
    case '1': if(!elevator.auto_mode && elevator.system_enabled) controlFan(true); break;
    case '0': if(!elevator.auto_mode) controlFan(false); break;
    case 't': printSystemStatus(); break;
    case 's': switchScreen(); break;
  }
}

void setup() {
  Serial.begin(115200);
  
  initializeHardware();
  setupWiFi();
  setupMQTT();
  
  Serial.println("🚀 Elevator Control System Started");
  calibrateMQ135();
}

void loop() {
  unsigned long currentMillis = millis();
  
  handleButtons();
  
  if (currentMillis - lastSensorUpdate > 500) {
    updateAirQualityLogic();
    updateLightingLogic();
    updateSystemIndicator();
    updateFanLED();
    lastSensorUpdate = currentMillis;
  }
  
  if (currentMillis - lastDisplayUpdate > 2000) {
    updateDisplay();
    lastDisplayUpdate = currentMillis;
  }
  
  if (currentMillis - lastFastPublish > FAST_PUBLISH_INTERVAL) {
    publishFastData();
    lastFastPublish = currentMillis;
  }

  if (currentMillis - lastSlowPublish > SLOW_PUBLISH_INTERVAL) {
    publishSlowData();
    lastSlowPublish = currentMillis;
  }
  
  if (currentMillis - lastScreenSwitch > 15000) {
    switchScreen();
    lastScreenSwitch = currentMillis;
  }
  
  handleMQTTLoop();
  
  if (Serial.available()) {
    handleSerialCommand(Serial.read());
  }
  
  delay(10); 
}
