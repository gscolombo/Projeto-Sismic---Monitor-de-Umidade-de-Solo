#include <led.h>

/**
 * Configura os pinos do LED RGB como saída digital e garante que
 * iniciem desligados.
 */
void setupLED(void) {
    LED_DIR |= (LED_RED_PIN | LED_GREEN_PIN | LED_BLUE_PIN);
    LED_OUT &= ~(LED_RED_PIN | LED_GREEN_PIN | LED_BLUE_PIN);
}

/**
 * Desliga todas as cores e aciona apenas o pino correspondente 
 * à cor solicitada.
 */
void setLEDColor(LedColor color) {
    // Apaga todas as cores antes de definir a nova
    LED_OUT &= ~(LED_RED_PIN | LED_GREEN_PIN | LED_BLUE_PIN);

    switch (color) {
        case LED_RED:
            LED_OUT |= LED_RED_PIN;
            break;
        case LED_GREEN:
            LED_OUT |= LED_GREEN_PIN;
            break;
        case LED_BLUE:
            LED_OUT |= LED_BLUE_PIN;
            break;
        case LED_OFF:
        default:
            break; 
    }
}
