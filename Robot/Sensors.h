enum LineStatus {
  LINE_NONE,
  LINE_STRAIGHT,
  LINE_LEFT_TURN,
  LINE_RIGHT_TURN,
  LINE_BIFURCATION
};

class Sensors {
public:
  int threshold;
  int dead_value;
  int sensors_count;
  int *sensor_pins;
  float *errors;
  float previous_error;

  Sensors(int threshold, int sensors_count, int *sensor_pins, float *errors, int dead_value) {
    this->threshold = threshold;
    this->sensor_pins = sensor_pins;
    this->sensors_count = sensors_count;
    this->errors = errors;
    this->previous_error = 0;
    this->dead_value = dead_value;
  }

  bool on_line(int sensor_pin) {
    int sensor_value = analogRead(sensor_pin);
    return sensor_value > threshold;
  }

  int get_line_position() {
    int line_position = 0;
    for (int i = 0; i < sensors_count; i++) {
      if (on_line(sensor_pins[i])) {
        line_position += (1 << i);
      }
    }
    return line_position;
  }

  LineStatus get_line_status() {
    int current_line_position = get_line_position();

    switch (current_line_position) {
      case 0b0000:
        return LINE_NONE;
      case 0b0110:
      case 0b0010:
      case 0b0100:
        return LINE_STRAIGHT;
      case 0b0001:
      case 0b0011:
      case 0b0111:
        return LINE_RIGHT_TURN;
      case 0b1000:
      case 0b1100:
      case 0b1110:
        return LINE_LEFT_TURN;
      case 0b1111:
      case 0b1001:
        return LINE_BIFURCATION;
      default:
        // Если неизвестный шаблон, обрабатываем как отсутствие линии
        return LINE_NONE;
    }
  }

  float calculate_error() {
    float error;
    int current_line_position = get_line_position();

    switch (current_line_position) {
      case 0b0001:
        error = errors[2];
        break;
      case 0b0011:
        error = errors[3];
        break;
      case 0b0111:
        error = errors[3];
        break;
      case 0b0010:
        error = -errors[1];
        break;
      case 0b0110:
        error = errors[0];
        break;
      case 0b1111:
        error = errors[0];
        break;
      case 0b0100:
        error = errors[1];
        break;
      case 0b1000:
        error = -errors[2];
        break;
      case 0b1100:
        error = -errors[3];
        break;
      case 0b1110:
        error = -errors[3];
        break;
      default:
        error = previous_error;
        break;
    }
    previous_error = error;
    return error;
  }

  bool is_dead(int sensor_pin) {
    int sensor_value = analogRead(sensor_pin);
    return sensor_value < dead_value;
  }

  // 1 - dead, 0 - alive
  int get_sensors_health() {
    int sensors_health = 0;
    for (int i = 0; i < sensors_count; i++) {
      if (is_dead(sensor_pins[i])) {
        sensors_health += (1 << i);
      }
    }
    return sensors_health;
  }
};