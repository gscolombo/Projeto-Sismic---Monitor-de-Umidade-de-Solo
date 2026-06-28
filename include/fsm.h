#ifndef FSM_H
#define FSM_H

#include <msp430.h>
#include <stdint.h>

/**
 * Estados da máquina de estados principal do firmware:
 *   START     - estado inicial, antes da primeira leitura do sensor;
 *   IDLE      - aguardando em baixo consumo entre leituras ou enquanto
 *               nenhum botão foi pressionado;
 *   WORK      - realiza a leitura/conversão da umidade e atualiza o LCD;
 *   CONFIG    - modo de configuração do limiar de alarme (threshold);
 *   LCD_ERROR - falha na inicialização do LCD (estado terminal).
 */
typedef enum State { START, IDLE, WORK, CONFIG, LCD_ERROR } State;

/** Flags de debounce: 1 enquanto o respectivo botão está em debounce. */
extern uint8_t s1Pressed, s2Pressed;

/** Estado atual e estado anterior da FSM (definidos em fsm.c). */
extern State state, prev_state;

/** Limiar de alarme de umidade (%), configurável pelo usuário em CONFIG. */
extern uint16_t threshold;

/** Configura os botões S1 (P2.1) e S2 (P1.1) e o Timer_A1 de debounce. */
void setupSwitches();

#endif
