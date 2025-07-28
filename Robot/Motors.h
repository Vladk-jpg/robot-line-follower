class Motors {
public:
  int normal_speed;
  int min_speed;
  int max_speed;

  int left_in1;
  int left_in2;

  int right_in1;
  int right_in2;

  Motors(int normal_speed, int min_speed, int max_speed,
         int left_in1, int left_in2,
         int right_in1, int right_in2) {

    this->normal_speed = normal_speed;
    this->min_speed = min_speed;
    this->max_speed = max_speed;

    this->left_in1 = left_in1;
    this->left_in2 = left_in2;

    this->right_in1 = right_in1;
    this->right_in2 = right_in2;
  }

  void initialize() {
    pinMode(left_in1, OUTPUT);
    pinMode(left_in2, OUTPUT);

    pinMode(right_in1, OUTPUT);
    pinMode(right_in2, OUTPUT);
  }

  void set_motor(int in1_pin, int in2_pin, int speed) {
    int normalizedSpeed = normalize_speed(speed);

    if (speed > 0) {                          // Движение вперед
      analogWrite(in1_pin, 0);                // Устанавливаем один пин в 0 (или LOW)
      analogWrite(in2_pin, normalizedSpeed);  // Подаем ШИМ на другой пин
    } else if (speed < 0) {                   // Движение назад
      analogWrite(in1_pin, normalizedSpeed);  // Подаем ШИМ на один пин
      analogWrite(in2_pin, 0);                // Устанавливаем другой пин в 0 (или LOW)
    } else {                                  // Остановка
      analogWrite(in1_pin, 0);
      analogWrite(in2_pin, 0);
    }
  }

  int normalize_speed(int speed) {
    return constrain(abs(speed), min_speed, max_speed);
  }

  void drive(int speed_difference, bool debug) {
    int left_speed = normal_speed + speed_difference;
    int right_speed = normal_speed - speed_difference;

    set_motor(left_in1, left_in2, left_speed);
    set_motor(right_in1, right_in2, right_speed);

    if (debug) {
      Serial.print("left_speed: ");
      Serial.print(normalize_speed(left_speed));
      Serial.print(" - ");
      Serial.println(left_speed > 0 ? "F" : "B");

      Serial.print("right_speed: ");
      Serial.print(normalize_speed(right_speed));
      Serial.print(" - ");
      Serial.println(right_speed > 0 ? "F" : "B");
    }
  }
};