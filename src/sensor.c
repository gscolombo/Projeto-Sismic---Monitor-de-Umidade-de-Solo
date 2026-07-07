/**
 * sensor.c
 *
 * Conversão da leitura bruta do ADC em percentual de umidade e lógica
 * de alarme de umidade baixa (comparação com o limiar configurável).
 * TODO:
 */

#include <buzzer.h>
#include <led.h>
#include <sensor.h>
#include <fsm.h>

/**
 * Converte a última leitura bruta do ADC (`value`, em [0, 4095]) em um
 * percentual de umidade, usando os limites de calibração DRY (solo
 * seco) e WET (solo encharcado) definidos em sensor.h.
 *
 * O cálculo é feito inteiramente em aritmética de inteiros, para
 * evitar uso de ponto flutuante:
 *   1. `value` é saturado ao intervalo [WET, DRY];
 *   2. `humidity_perc` representa o percentual em décimos de ponto
 *      percentual, escalado por 1000 (ex.: 453 -> 45.3%);
 *   3. p.i e p.d são, respectivamente, a parte inteira e o primeiro
 *      dígito decimal do percentual.
 *
 * Quanto menor o `value` (maior tensão lida = solo mais úmido), maior
 * o percentual de umidade retornado — por isso a inversão (DRY - value).
 */

Percent convertRead() {
  Percent p;
  if (value > DRY)
    value = DRY; // Satura leituras acima do valor de referência "seco"
  if (value < WET)
    value = WET; // Satura leituras abaixo do valor de referência "encharcado"

  // Distância do valor lido ao extremo seco, escalada por 1000 para
  // preservar precisão decimal na divisão inteira a seguir.
  unsigned long humidity = ((unsigned long)(DRY - value)) * 1000;

  // Percentual de umidade (x1000), normalizado pela faixa de calibração.
  unsigned long humidity_perc = humidity / (DRY - WET);

  // DEN = 6554 (~65536/10): multiplicar e deslocar 16 bits aproxima
  // uma divisão por 10, extraindo a parte inteira do percentual.
  p.i = (humidity_perc * DEN) >> 16;
  p.d = humidity_perc - 10 * p.i; // Primeiro dígito decimal restante

  return p;
}

/**
 * Verifica se o percentual de umidade atual está abaixo do limiar de
 * alarme `t` e, em caso afirmativo, exibe uma mensagem de alerta na
 * segunda linha do LCD (posição 0x40). A mensagem alterna entre
 * "Alerta!" e "Baixa umidade!" a cada chamada em que o alarme
 * permanece ativo, para chamar mais atenção do usuário.
 *
 * Quando a umidade está acima do limiar, apenas garante que a linha de
 * alerta fique limpa.
 */
void checkThreshold(Percent *p, unsigned int t) {
  static char *alert_message = "Alerta!";
  static uint8_t buzzer_on = 0;

  // TODO: Considerar média/mediana móvel de percentual de umidade para disparo
  // de alarme
  if (p->i < t) {
    clearLine(1);
    LCDWrite(alert_message, 0x40);

    // Alterna a mensagem de alerta exibida na próxima chamada.
    if (!strcmp(alert_message, "Alerta!"))
      alert_message = "Baixa umidade!";
    else
      alert_message = "Alerta!";
    setLEDColor(LED_RED); // Acende o LED em Vermelho
    if (buzzer_on && !buzzerStop)
      buzzerOn(); // Ativa o alarme sonoro do Buzzer
    else
      buzzerOff();

    buzzer_on ^= 1;
  } else {
    clearLine(1);   // Umidade OK: garante que a linha de alerta fique limpa
    buzzerOff();    // Garante que o Buzzer permaneça desligado fora do estado
                    // crítico
    buzzerStop = 0; // Reseta a flag de desligamento manual do alarme

    // Verifica se a umidade está na faixa de saturação (ex: >= 85%)
    if (p->i >= 85) {
      setLEDColor(LED_BLUE); // Acende o LED em Azul (Saturado)
    } else {
      setLEDColor(LED_GREEN); // Acende o LED em Verde (Ideal)
    }
  }
}
