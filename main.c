//=============================================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
//=============================================================================
#include "pins.h"
#include "events.h"
#include "rtos.h"
#include "i2csw.h"
#include "rc5.h"
#include "enc.h"
#include "kbd.h"
#include "ds18x20.h"
#include "bmp180.h"
#include "dht22.h"
#include "rda5807.h"
#include "lcd_lib.h"
#include "ds3231.h"
#include "interface.h"
#include "utils.h"
//=============================================================================
int main(void)
{
  LIGHT_init();
  LCD_init();
  LIGHT_on();
  BEEP_init();
  DDR(PORT_LIGHT) |= PORT_LIGHT_LINE;
  PORT(PORT_LIGHT) |= PORT_LIGHT_LINE;
  ds18x20SearchDevices();
  RTC_init();
  RTOS_init();
  RC5_Init();
  RC5_Reset();
  ENC_init();
  KBD_init();
  bmp180Init();
  dht22Init();
  rda5807Init();
  rda5807PowerOn();
  rda5807SetMute(1);
  //RTC_set_time(13,41, 0);
  LCD_clear();
  LCD_goto(0, 0);
  LCD_puts("POGODNAY STATION");
  LCD_goto(0, 1);
  LCD_puts("  VERSION 1.0   ");
  _delay_ms(1000);
  LCD_clear();
  LCD_load_bignum();
  ds18x20Process();
  if (bmp180HaveSensor()) bmp180Convert();
  dht22Read();
#if (DEBUG == 1)
  if ((BUT_1_PINX & (1<<(BUT_1_PIN))) == 0) {
    _delay_ms(100);
    if ((BUT_1_PINX & (1<<(BUT_1_PIN))) == 0) {
      RTOS_setTask(EVENT_SET_STATE_OPTION, 0, 0);
      BEEP_beep();
	  while (((BUT_1_PINX & (1<<(BUT_1_PIN))) == 0)) { }
      BEEP_beep();
    }
  }
#else
  RTOS_setTask(EVENT_SET_STATE_OPTION, 0, 0);
  BEEP_beep();
#endif
  RTOS_setTaskFunc(set_blink, 0, 1000);      // моргание
  RTOS_setTaskFunc(KBD_scan, 0, 5);          // запускаем опрос кнопок
  RTOS_setTaskFunc(ENC_poll, 0, 1);          // запускаем опрос енкодера
  RTOS_setTaskFunc(RC5_scan, 0, 5);          // запускаем опрос RC5
  sei(); 				        			 // Разрешили прерывания
  while(1) {
    RTOS_dispatchTask();    			     // Вызываем диспетчер в цикле.    
  }
}
//=============================================================================
