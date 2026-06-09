#include <sensor.h>

Percent convertRead() {
  Percent p;

  unsigned long humidity =
      ((unsigned long)(4095 - analog_read_value) + 1) * 1000;

  unsigned long humidity_perc = humidity >> 12; // Divide por 4096

  p.i = (humidity_perc * DEN) >> 16;
  p.d = humidity_perc - 10 * p.i;

  return p;
}

void checkThreshold(Percent *p, unsigned int t) {
  static char *alert_message = "Alerta!";

  if (p->i < t) {
    clearLine(1);
    LCDWrite(alert_message, 0x40);

    if (!strcmp(alert_message, "Alerta!"))
      alert_message = "Baixa umidade!";
    else
      alert_message = "Alerta!";
  } else
    clearLine(1);
}
