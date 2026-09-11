#pragma once // Захист від подвійного підключення файлу

// ==================== WIFI & MQTT ====================
const char* ssid = "";
const char* password = "";
const char* mqtt_server = ""; 
const int mqtt_port = 1883;

// ==================== ПІНИ ====================
#define MQ135_PIN 4
#define RELAY_PIN 2
#define MOTION_SENSOR_PIN 5
#define BUTTON_PIN 13
#define FAN_BUTTON_PIN 9
#define WHITE_LED_PIN 12
#define FAN_LED_PIN 21
#define RGB_RED 18
#define RGB_GREEN 17  
#define RGB_BLUE 16

// ==================== ЕКРАН ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define SCL_PIN 15
#define SDA_PIN 14

// ==================== ТАЙМІНГИ ====================
const unsigned long LIGHT_DURATION = 15000; 
const unsigned long FAST_PUBLISH_INTERVAL = 2000;
const unsigned long SLOW_PUBLISH_INTERVAL = 10000;
