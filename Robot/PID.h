class PID {
public:
  float KP;
  float KI;
  float KD;

  float last_proportional = 0; 
  float integral = 0;

  PID(float KP, float KI, float KD) {
    this->KP = KP;
    this->KD = KD;
    this->KI = KI;
  }

  int calculate_speed_difference(float current_error) {
    float proportional_term = current_error * KP;
    integral += current_error; 

    float integral_term = integral * KI;
    float derivative = current_error - last_proportional; 
    float derivative_term = derivative * KD;

    last_proportional = current_error; 

    return int(proportional_term + integral_term + derivative_term);
  }
};