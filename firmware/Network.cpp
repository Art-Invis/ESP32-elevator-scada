#include "Network.h"
#include "Config.h"
#include "SystemState.h"
#include "Hardware.h"
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);

const char* topic_elevator_status = "home/elevator/status";
const char* topic_motion_events = "home/elevator/motion";
const char* topic_commands = "home/elevator/commands";

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  if (String(topic) == topic_commands) {
    if (message == "fan_on") controlFan(true);
    else if (message == "fan_off") controlFan(false);
    else if (message == "auto_mode") elevator.auto_mode = true;
    else if (message == "manual_mode") elevator.auto_mode = false;
    else if (message == "system_on") elevator.system_enabled = true;
    else if (message == "system_off") { 
      elevator.system_enabled = false;
      controlFan(false);
      controlLight(false);
    }
    else if (message == "light_on") controlLight(true);
    else if (message == "light_off") controlLight(false);
    else if (message.startsWith("threshold:")) {
      elevator.threshold = message.substring(10).toInt();
    }
    publishFastData(); // Відразу надсилаємо оновлений статус
  }
}

void setupWiFi() {
  String hostname = "ESP32-ELEVATOR-" + String(random(0xffff), HEX);
  WiFi.setHostname(hostname.c_str()); 
  
  Serial.print("📶 Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" ✅");
    Serial.print("📱 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(" ❌");
  }
}

void setupMQTT() {
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setBufferSize(1024);
}

void handleMQTTLoop() {
  if (!mqttClient.connected()) {
    if (WiFi.status() == WL_CONNECTED) {
      String clientId = "Elevator-ESP32-" + String(random(0xffff), HEX);
      if (mqttClient.connect(clientId.c_str())) {
        mqttClient.subscribe(topic_commands, 1);
      }
    }
  } else {
    mqttClient.loop();
  }
}

void publishFastData() {
  if (!mqttClient.connected()) return;

  String jsonData = "{";
  jsonData += "\"co2\":" + String(elevator.co2_ppm, 0);
  jsonData += ",\"quality_level\":\"" + getQualityString(elevator.air_quality) + "\"";
  jsonData += ",\"fan_running\":" + String(elevator.fan_running ? "true" : "false");
  jsonData += ",\"light_running\":" + String(elevator.light_running ? "true" : "false");
  jsonData += ",\"motion_detected\":" + String(elevator.motion_detected ? "true" : "false");
  jsonData += "}";

  mqttClient.publish(topic_elevator_status, (const uint8_t*)jsonData.c_str(), jsonData.length(), true); 
}

void publishSlowData() {
  if (!mqttClient.connected()) return;

  String jsonData = "{";
  jsonData += "\"motion_per_min\":" + String(motionCount);
  jsonData += ",\"system_enabled\":" + String(elevator.system_enabled ? "true" : "false");
  jsonData += ",\"auto_mode\":" + String(elevator.auto_mode ? "true" : "false");
  jsonData += ",\"threshold\":" + String(elevator.threshold);
  jsonData += ",\"fan_cycles\":" + String(fanCycles);
  jsonData += ",\"fan_on_time\":" + String(fanOnTime / 1000);
  jsonData += "}";

  mqttClient.publish(topic_elevator_status, (const uint8_t*)jsonData.c_str(), jsonData.length(), true);
  motionCount = 0; // Скидаємо лічильник для статистики
}

void publishMotionEvent(bool detected) {
  if (!mqttClient.connected()) return;
  String motionData = "{\"motion\":" + String(detected ? "true" : "false") + 
                      ",\"timestamp\":" + String(millis()) + "}";
  mqttClient.publish(topic_motion_events, (const uint8_t*)motionData.c_str(), motionData.length(), 1); 
}
