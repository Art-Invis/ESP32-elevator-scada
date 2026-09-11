#pragma once

enum AirQualityLevel {
  EXCELLENT, GOOD, MODERATE, POOR, DANGEROUS
};

struct ElevatorState {
  float co2_ppm;
  AirQualityLevel air_quality;
  bool fan_running;
  bool motion_detected;
  bool light_running;
  unsigned long light_timeout;
  bool system_enabled;
  bool auto_mode;
  int threshold;
  bool alarm_displayed;
};

extern ElevatorState elevator;
extern unsigned long motionCount;
extern int fanCycles;
extern unsigned long fanOnTime;
