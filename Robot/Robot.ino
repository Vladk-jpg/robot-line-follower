// ================ SETTINGS ================
#define SENSORS_COUNT 4

#define MAX_SPEED 255
#define NORMAL_SPEED 205
#define MIN_SPEED 0

#define LEFT_IN1 3
#define LEFT_IN2 5
#define RIGHT_IN1 6
#define RIGHT_IN2 9

#define THRESHOLD 50

const int SENSOR_PINS[SENSORS_COUNT] = { A0, A1, A2, A3 };
const int ERRORS[4] = { 0, 1, 2, 3 };

const float KP = 80;
const float KI = 0;
const float KD = 50;

const bool DEBUG = true;
const int DELAY_TIME = 100;

// ================ ======== ================

#include "Sensors.h"
#include "Motors.h"
#include "PID.h"

Sensors sensors(THRESHOLD, SENSORS_COUNT, SENSOR_PINS, ERRORS);

Motors motors(
  NORMAL_SPEED, MIN_SPEED, MAX_SPEED,
  LEFT_IN1, LEFT_IN2,
  RIGHT_IN1, RIGHT_IN2);

PID pid(KP, KI, KD);

void setup() {
  Serial.begin(9600);
  motors.initialize();
}

void loop() {
  int error = sensors.calculate_error();
  int speed_difference = pid.calculate_speed_difference(error);
  motors.drive(speed_difference, DEBUG);

  if (DEBUG) {
    Serial.print("error: ");
    Serial.println(error);
    Serial.print("speed_difference: ");
    Serial.println(speed_difference);
    Serial.println("--------------------------------------");
    delay(DELAY_TIME);
  }
}
