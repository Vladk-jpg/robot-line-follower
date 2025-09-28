// ================ SETTINGS ================
#define SENSORS_COUNT 4

#define MAX_SPEED 150
#define NORMAL_SPEED 130
#define TURN_SPEED 80
#define MIN_SPEED 0

#define LEFT_IN1 6
#define LEFT_IN2 5
#define RIGHT_IN1 10
#define RIGHT_IN2 9

#define THRESHOLD 200
#define DEAD_VALUE 10

#define IR_PIN 2

const int SENSOR_PINS[SENSORS_COUNT] = { A3, A2, A1, A0 };
const float ERRORS[4] = { 0, 1, 3, 5 };
const int SENSORS_LEDS[SENSORS_COUNT] = { 12, 8, 7, 4 };

const float KP = 15;
const float KI = 0;
const float KD = 10;

const bool DEBUG = false;
const int DELAY_TIME = 100;

unsigned long last_line_seen_time = 0;
bool line_currently_lost = false;
const unsigned long LINE_LOSS_TIMEOUT_MS = 2000;

unsigned long time_from_turn = 0;
bool current_turn = false;
int turn_direction = 0;
const unsigned long TURN_TIMEOUT_MS = 500;

// ================ ======== ================

#include <IRremote.hpp>
#include "Sensors.h"
#include "Motors.h"
#include "PID.h"

IRrecv irReceiver(IR_PIN);

Sensors sensors(THRESHOLD, SENSORS_COUNT, SENSOR_PINS, ERRORS, DEAD_VALUE);

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
  int health = sensors.get_sensors_health();

  LineStatus current_line_status = sensors.get_line_status();

  if (irReceiver.available()) {  // Проверяем, есть ли в буфере данные
    irReceiver.decode();         // Декодируем сигнал
    unsigned long code = irReceiver.decodedIRData.decodedRawData;
    Serial.println(code, HEX);

    if (code == 0xE718FF00) {  //Forward
      stop_flag = false;       // Сброс флага остановки
      line_currently_lost = false;
      motors.drive(NORMAL_SPEED, 0, DEBUG);
    } else if (code == 0xAD52FF00) {  //Back
      stop_flag = false;
      motors.drive(-NORMAL_SPEED, 0, DEBUG);
      delay(2000);
    } else if (code == 0xA55AFF00) {  //Right
      stop_flag = false;
      current_turn = true;
      time_from_turn = millis();
      turn_direction = 1;
    } else if (code == 0xF708FF00) {  //Left
      stop_flag = false;
      current_turn = true;
      time_from_turn = millis();
      turn_direction = -1;
    } else if (code == 0xE31CFF00) {  //Stop
      stop_flag = true;
    } else {
      // Логика для других кнопок
    }
    irReceiver.resume();  // Возобновляем прием следующего сигнала
  }

  // Общий флаг остановки, срабатывает от пульта или аварийной ситуации
  if (stop_flag) {
    motors.drive(MIN_SPEED, 0, DEBUG);
    // Останавливаем все операции, кроме приема IR-сигнала
    return;
  }

  // Индикация здоровья датчиков
  for (int i = 0; i < SENSORS_COUNT; ++i) {
    boolean is_dead = bitRead(health, i);
    digitalWrite(SENSORS_LEDS[i], is_dead);
  }

  // Главная логика следования по линии и восстановления
  switch (current_line_status) {

    case LINE_BIFURCATION:
      // При обнаружении развилки
      current_turn = false; 
      stop_flag = true;
      motors.drive(MIN_SPEED, 0, DEBUG);

      if (DEBUG) Serial.println("A fork has been detected! Stop.");
      break;

    case LINE_NONE:
      if (!line_currently_lost) {
        last_line_seen_time = millis();
        line_currently_lost = true;
      }

      // Проверка таймаута потери линии
      if (millis() - last_line_seen_time > LINE_LOSS_TIMEOUT_MS) {
        // Линия потеряна слишком долго - полная остановка
        motors.drive(MIN_SPEED, 0, DEBUG);
        if (DEBUG) Serial.println("The line is not visible for too long! Stop.");
        current_turn = false;

      } else {
        // Линия только что потеряна - активируем непрерывный поворот
        if (!current_turn && turn_direction != 0) {
          current_turn = true;
          time_from_turn = millis();
          if (DEBUG) Serial.println("Line lost. Initiating continuous turn.");
        }

        if (current_turn) {
          if (millis() - time_from_turn > TURN_TIMEOUT_MS) {
            if (DEBUG) Serial.println("Continuous turn timed out! Reverting to LINE_NONE logic.");
            current_turn = false;
          }

          if (current_turn) {
            // Выполнение резкого поворотного движения (Pivot Turn)
            if (turn_direction == 1) {  // Поворот направо
              motors.drive(TURN_SPEED, TURN_SPEED * 2 / 3, DEBUG);
            } else if (turn_direction == -1) {  // Поворот налево
              motors.drive(TURN_SPEED, -TURN_SPEED * 2 / 3, DEBUG);
            } else {
              motors.drive(NORMAL_SPEED, speed_difference, DEBUG);
            }
          }
        }
        if (!current_turn) {
          // Нет заданного направления, или таймаут истек - едем по PID
          motors.drive(NORMAL_SPEED, speed_difference, DEBUG);
        }
      }
      break;

    case LINE_STRAIGHT:
      if (current_turn) {
        const int ALIGNMENT_DIFF = 40;
        int correction_speed_difference = -turn_direction * ALIGNMENT_DIFF; 
        motors.drive(NORMAL_SPEED, correction_speed_difference, DEBUG); 
        current_turn = false;
        turn_direction = 0; 
        if (DEBUG) Serial.println("Line found. Applied opposite correction.");
      } else {
         motors.drive(NORMAL_SPEED, speed_difference, DEBUG);
      }
      
      line_currently_lost = false;
      break;

    case LINE_LEFT_TURN:
      if (turn_direction == 1 && (millis() - time_from_turn < TURN_TIMEOUT_MS)) {
        motors.drive(MIN_SPEED, 0, DEBUG);
        if (DEBUG) Serial.println("Opposite line detected during turn! Emergency Stop.");
        current_turn = false;
        stop_flag = true;
        break;
      }

      line_currently_lost = false;
      current_turn = false;  // Отменить состояние поворота при обнаружении линии
      turn_direction = -1;   // Запоминаем последнее направление
      motors.drive(NORMAL_SPEED, speed_difference, DEBUG);
      break;

    case LINE_RIGHT_TURN:
      if (turn_direction == -1 && (millis() - time_from_turn < TURN_TIMEOUT_MS)) {
        motors.drive(MIN_SPEED, 0, DEBUG);
        if (DEBUG) Serial.println("Opposite line detected during turn! Emergency Stop.");
        current_turn = false;
        stop_flag = true;
        break;
      }

      line_currently_lost = false;
      current_turn = false;  // Отменить состояние поворота при обнаружении линии
      turn_direction = 1;    // Запоминаем последнее направление
      motors.drive(NORMAL_SPEED, speed_difference, DEBUG);
      break;

    default:
      current_turn = false;  // Если любой другой паттерн (например, 0b1001), выходим из поворота
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
