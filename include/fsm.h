#ifndef FSM_H
#define FSM_H

#include <msp430.h>
#include <stdint.h>

typedef enum State { START, IDLE, WORK, CONFIG, LCD_ERROR } State;

extern uint8_t s1Pressed, s2Pressed;

extern State state, prev_state;
extern uint16_t threshold;

void setupSwitches();

#endif
