#ifndef SENSOR_H
#define SENSOR_H

#include <adc.h>
#include <lcd.h>

#include <stdio.h>
#include <string.h>

#define DEN 6554 // ~65536/10, usado para extrair a parte inteira do percentual via shift (ver convertRead)
#define WET 1200 // Leitura bruta do ADC com o sensor em solo encharcado (calibração)
#define DRY 4000 // Leitura bruta do ADC com o sensor em solo seco (calibração)

/** Percentual de umidade: p.i = parte inteira, p.d = primeiro dígito decimal. */
typedef struct Percent {
  unsigned int i;
  unsigned int d;
} Percent;

/** Converte a última leitura do ADC (`value`) em percentual de umidade. */
Percent convertRead();

/** Compara o percentual de umidade com o limiar `t` e atualiza o alarme no LCD. */
void checkThreshold(Percent *, unsigned int);

// TODO: Calcular média/mediana móvel do percentual de umidade

#endif
