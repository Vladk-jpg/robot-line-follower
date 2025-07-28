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

unsigned long last_line_seen_time = 0;
bool line_currently_lost = false;
const unsigned long LINE_LOSS_TIMEOUT_MS = 2000;

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

  LineStatus current_line_status = sensors.get_line_status();

  switch (current_line_status) {
    case LINE_BIFURCATION:
      motors.drive(0, DEBUG);
      if (DEBUG) Serial.println("A fork has been detected! Stop.");
      // while (true) {};
      break;

    case LINE_NONE:
      if (!line_currently_lost) {
        last_line_seen_time = millis();
        line_currently_lost = true;
      }

      if (millis() - last_line_seen_time > LINE_LOSS_TIMEOUT_MS) {
        motors.drive(0, DEBUG);
        Serial.println("The line is not visible for too long! Stop.");
        // while (true) {};
      } else {
        // Если линия временно потеряна, продолжаем движение с последним рассчитанным
        // отклонением. ПИД-контроллер будет использовать previous_error из Sensors.
        motors.drive(speed_difference, DEBUG);
      }
      break;

    case LINE_STRAIGHT:
    case LINE_LEFT_TURN:
    case LINE_RIGHT_TURN:
      line_currently_lost = false;
      motors.drive(speed_difference, BEBUG);
      break;
    default:
      motors.drive(speed_difference, DEBUG);
      line_currently_lost = false;
      break;
  }

  if (DEBUG) {
    Serial.print("error: ");
    Serial.println(error);
    Serial.print("speed_difference: ");
    Serial.println(speed_difference);
    Serial.print("Line Status: ");
    switch (current_line_status) {
      case LINE_NONE: Serial.println("NONE"); break;
      case LINE_STRAIGHT: Serial.println("STRAIGHT"); break;
      case LINE_LEFT_TURN: Serial.println("LEFT TURN"); break;
      case LINE_RIGHT_TURN: Serial.println("RIGHT TURN"); break;
      case LINE_BIFURCATION: Serial.println("BIFURCATION"); break;
    }
    Serial.println("--------------------------------------");
    delay(DELAY_TIME);
  }
  delay(10);
}
