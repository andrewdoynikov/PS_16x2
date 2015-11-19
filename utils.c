#include <avr/io.h>
#include <util/delay.h>
#include "lcd_lib.h"
#include "utils.h"
//=============================================================================
void BEEP_init(void)
{
BEEP_DDR |= (1 << BEEP_PIN);  // пин на выход
BEEP_PORT &= ~(1 << BEEP_PIN); // звук выключен
}
//=============================================================================
void BEEP_beep(void)
{
BEEP_PORT |= (1 << BEEP_PIN);  // звук выключен
_delay_ms(BEEP_TIME);
BEEP_PORT &= ~(1 << BEEP_PIN); // звук включен
}
//=============================================================================
void BEEP_long(void)
{
BEEP_PORT |= (1 << BEEP_PIN);  // звук выключен
_delay_ms(BEEP_LONG_TIME);
BEEP_PORT &= ~(1 << BEEP_PIN); // звук включен
}
//=============================================================================
void print_dec(unsigned int x, unsigned char n, unsigned char fillch)
{
unsigned char i;
unsigned char s[10];
for (i = 0; i < n; i++) {
  s[n - i - 1] = '0' + (x % 10);
  x /= 10;
}
for (i=0; i<(n - 1); i++) {
  if (s[i] == '0') s[i] = fillch; else break;
}
  for (i=0; i<n; i++) LCD_dat(s[i]);
}
//=============================================================================
