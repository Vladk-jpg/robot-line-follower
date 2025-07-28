class Sensors {
public:
  int threshold;
  int sensors_count;
  int *sensor_pins;
  int *errors;
  int previous_error;

  Sensors(int threshold, int sensors_count, int *sensor_pins, int *errors) {
    this->threshold = threshold;
    this->sensor_pins = sensor_pins;
    this->sensors_count = sensors_count;
    this->errors = errors;
    this->previous_error = 0;
  }

  bool on_line(int sensor_pin) {
    int sensor_value = analogRead(sensor_pin);
    return sensor_value < threshold;
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

  int calculate_error() {
    int error;
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
        error = errors[1];
        break;
      case 0b0110:
        error = errors[0];
        break;
      case 0b1111:
        error = errors[0];
        break;
      case 0b0100:
        error = -errors[1];
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
};