#pragma once
#include <Arduino.h>
#include "SystemState.h"

void initializeHardware();
void initializeOLED();
void startupSequence();
void setRGBColor(int red, int green, int blue);
void updateFanLED();
void updateSystemIndicator();
void controlFan(bool state);
void controlLight(bool state);
void handleButtons();
void calibrateMQ135();
float getResistance();
float getPPM();
void updateDisplay();
void switchScreen();
void printSystemStatus();
String getQualityString(AirQualityLevel q);
