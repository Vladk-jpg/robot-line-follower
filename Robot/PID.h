class PID {
public:
  float KP;
  float KI;
  float KD;

  float last_proportional = 0, integral = 0;

  PID(float KP, float KI, float KD) {
    this->KP = KP;
    this->KD = KD;
    this->KI = KI;
  }

  int calculate_speed_difference(float proportional) {
    float derivative = proportional / 2 - last_proportional;
    integral = integral + proportional / 2;
    last_proportional = proportional / 2;

    return int(proportional * KP + integral * KI + derivative * KD);
  }
};
