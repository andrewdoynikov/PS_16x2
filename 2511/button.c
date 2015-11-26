#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
//=============================================================================
#include "button.h"
#include "events.h"
#include "rtos.h"
#include "pins.h"
//=============================================================================
void KBD_init(void)
{
  DDR(BEEPER) |= BEEPER_LINE;
  PORT(BEEPER) &= ~BEEPER_LINE;
  DDR(BUTTON_1) &= ~BUTTON_1_LINE;
  PORT(BUTTON_1) |= BUTTON_1_LINE;
}

//=============================================================================
void KBD_poll(void)
{
static uint16_t state1 = 0; //holds present state
static uint16_t cntLong = 0; 
state1 = (state1 << 1) | (! bit_is_clear(PIN(BUTTON_SET), BUTTON_SET_LINE)) | 0xE000;
if (state1 == 0xF000) {
  if (bit_is_clear(PIN(BUTTON_SET), BUTTON_SET_LINE)) {
    cntLong++; 
    if (cntLong >= LONG_PRESS) {
	  state1 = 0;
      cntLong = 0;
      RTOS_setTask(EVENT_KEY_SET_LONG, 0, 0);
    }
  } else {
    RTOS_setTask(EVENT_KEY_SET, 0, 0);
  }
}
}
//=============================================================================


