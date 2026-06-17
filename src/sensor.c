#include <sensor.h>

Percent convertRead() {
  Percent p;
  if (value > DRY)
    value = DRY;
  if (value < WET)
    value = WET;

  unsigned long humidity = ((unsigned long)(DRY - value)) * 1000;

  unsigned long humidity_perc = humidity / (DRY - WET);

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
