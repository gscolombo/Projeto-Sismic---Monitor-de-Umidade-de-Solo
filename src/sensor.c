#include <sensor.h>

void parseRead(char *buf) {
  unsigned long humidity =
      ((unsigned long)(4095 - analog_read_value) + 1) * 1000;

  unsigned long humidity_perc = humidity >> 12; // Divide por 4096

  unsigned int i = (humidity_perc * DEN) >> 16;
  unsigned int d = humidity_perc - 10 * i;

  sprintf(buf, "Umidade: %d.%d%%", i, d);
}
