#pragma once
#include <Arduino.h>

void setupWiFi();
void setupMQTT();
void handleMQTTLoop();
void publishFastData();
void publishSlowData();
void publishMotionEvent(bool detected);
