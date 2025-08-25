// ================ SETTINGS ================
#define SENSORS_COUNT 4

#define MAX_SPEED 255
#define NORMAL_SPEED 205
#define TURN_SPEED 100
#define MIN_SPEED 0

#define LEFT_IN1 6
#define LEFT_IN2 5
#define RIGHT_IN1 10
#define RIGHT_IN2 9

#define THRESHOLD 300

#define IR_PIN 2

const int SENSOR_PINS[SENSORS_COUNT] = { A3, A2, A1, A0 };
const float ERRORS[4] = { 0, 0.5, 2, 3 };

const float KP = 40;
const float KI = 0;
const float KD = 30;

const bool DEBUG = false;
const int DELAY_TIME = 100;

unsigned long last_line_seen_time = 0;
bool line_currently_lost = false;
const unsigned long LINE_LOSS_TIMEOUT_MS = 2000;

// ================ ======== ================

#include <IRremote.hpp>
#include "Sensors.h"
#include "Motors.h"
#include "PID.h"

IRrecv irReceiver(IR_PIN);

Sensors sensors(THRESHOLD, SENSORS_COUNT, SENSOR_PINS, ERRORS);

Motors motors(
  MIN_SPEED, MAX_SPEED,
  LEFT_IN1, LEFT_IN2,
  RIGHT_IN1, RIGHT_IN2);

PID pid(KP, KI, KD);

bool stop_flag = false;

void setup() {
  Serial.begin(9600);
  motors.initialize();
  irReceiver.enableIRIn();
}

void loop() {
  int error = sensors.calculate_error();
  int speed_difference = pid.calculate_speed_difference(error);

  LineStatus current_line_status = sensors.get_line_status();

  if (irReceiver.available()) {  // Проверяем, есть ли в буфере данные
    irReceiver.decode();         // Декодируем сигнал
    unsigned long code = irReceiver.decodedIRData.decodedRawData;
    Serial.println(code, HEX);

    if (code == 0xE718FF00) {  //Forward
      line_currently_lost = false;
      motors.drive(NORMAL_SPEED, 0, DEBUG);
    } else if (code == 0xAD52FF00) {  //Back
      motors.drive(-NORMAL_SPEED, 0, DEBUG);
      delay(2000);
    } else if (code == 0xA55AFF00) {  //Right
      motors.drive(TURN_SPEED, 50, DEBUG);
      delay(500);
    } else if (code == 0xF708FF00) {  //Left
      motors.drive(TURN_SPEED, -50, DEBUG);
      delay(500);
    } else if (code == 0xE31CFF00) {  //Stop

    } else {
      // Логика для других кнопок
    }
    irReceiver.resume();  // Возобновляем прием следующего сигнала
  }

  switch (current_line_status) {
    case LINE_BIFURCATION:
      motors.drive(MIN_SPEED, 0, DEBUG);
      if (DEBUG) Serial.println("A fork has been detected! Stop.");

      break;

    case LINE_NONE:
      if (!line_currently_lost) {
        last_line_seen_time = millis();
        line_currently_lost = true;
      }

      if (millis() - last_line_seen_time > LINE_LOSS_TIMEOUT_MS) {
        motors.drive(MIN_SPEED, 0, DEBUG);
        if (DEBUG) Serial.println("The line is not visible for too long! Stop.");

      } else {
        motors.drive(NORMAL_SPEED, speed_difference, DEBUG);
      }
      break;

    case LINE_STRAIGHT:
    case LINE_LEFT_TURN:
    case LINE_RIGHT_TURN:
      line_currently_lost = false;
      motors.drive(NORMAL_SPEED, speed_difference, DEBUG);
      break;
    default:
      motors.drive(NORMAL_SPEED, speed_difference, DEBUG);
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
}
