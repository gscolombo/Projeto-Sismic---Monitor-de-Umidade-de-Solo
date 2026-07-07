#include <buzzer.h>

/**
 * Configura o pino do Buzzer como saída digital e inicia desligado.
 */
void setupBuzzer(void) {
    BUZZER_DIR |= BUZZER_PIN;
    BUZZER_OUT &= ~BUZZER_PIN;
}

/**
 * Ativa o alarme sonoro (nível lógico alto).
 */
inline void buzzerOn(void) {
    BUZZER_OUT |= BUZZER_PIN;
}

/**
 * Desativa o alarme sonoro (nível lógico baixo).
 */
inline void buzzerOff(void) {
    BUZZER_OUT &= ~BUZZER_PIN;
}
