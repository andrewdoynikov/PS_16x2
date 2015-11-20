#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
//=============================================================================
#include "pins.h"
#include "rtos.h"
#include "events.h"
#include "lcd_lib.h"
#include "rc5.h"
#include "ds18x20.h"
#include "bmp180.h"
#include "dht22.h"
#include "ds3231.h"
#include "rda5807.h"
#include "button.h"
#include "interface.h"
#include "eeproms.h"
#include "utils.h"
//=============================================================================
#define OMENU_MAX 2
char *optionmenu[] = {
"Set TIME  ", 
"Set DATE  ", 
"Set ALARM " 
};
#define OM_SETTIME     	0
#define OM_SETDATE     	1
#define OM_SETALARM    	2
uint8_t o_menu = OM_SETTIME;
//=============================================================================
#define DEBUGER		0
#define SHOW_TIME_SENSOR_NUM	1000
#define SHOW_TIME_SENSOR_TEMP	5000
//=============================================================================
unsigned char a_onoff, a_hour, a_min, a_sec;
unsigned char n_edit_time = 0, n_edit_date = 0, n_edit_alarm = 0;
unsigned char hour, min, sec;
unsigned char wday, day, mes, year;
//=============================================================================
extern void (*pState)(unsigned char event);
//=============================================================================
uint16_t freqs = 10000;
#define SET_STATE(a) pState = a  // макрос для смены состояния
unsigned char blinks = 0;
unsigned char sensor_num = 0;
unsigned char mute = 0;
unsigned char regim = 0;
char *den_dw[] = {"MO","TU","WE","TH","FR","SA","SU"};
char *den_dw_full[] = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday", "Sunday"};
char *mes_full[] = {"January","February","March","April","May","June","July","August","September","October","November","December"};
char bignumchars1[] = {
  0x03,0x00,0x03, 0x00,0x03,0x20, 0x02,0x02,0x03, 0x00,0x02,0x03, 0x03,0x01,0x03,
  0x03,0x02,0x02, 0x03,0x02,0x02, 0x00,0x00,0x03, 0x03,0x02,0x03, 0x03,0x02,0x03
};
char bignumchars2[] = {
  0x03,0x01,0x03, 0x01,0x03,0x01, 0x03,0x01,0x01, 0x01,0x01,0x03, 0x20,0x20,0x03,
  0x01,0x01,0x03, 0x03,0x01,0x03, 0x20,0x20,0x03, 0x03,0x01,0x03, 0x01,0x01,0x03
};
//=============================================================================
void lcd_bigchar(unsigned char col, unsigned char x)
{
  unsigned char i;
  i = x * 3;
  LCD_goto(col, 0);
  LCD_dat(bignumchars1[i]); LCD_dat(bignumchars1[i + 1]); LCD_dat(bignumchars1[i + 2]); LCD_dat(' ');
  LCD_goto(col, 1);
  LCD_dat(bignumchars2[i]); LCD_dat(bignumchars2[i + 1]); LCD_dat(bignumchars2[i + 2]); LCD_dat(' ');
}
//=============================================================================
void show_bigtime(void)
{
  unsigned char h, m, s;
  RTC_get_time(&h, &m, &s);
  LCD_goto(0, 0);
  if (blinks)
  { 
    LCD_goto(8, 0); LCD_dat(0x04);
    LCD_goto(8, 1); LCD_dat(0x04);
  } else {
    LCD_goto(8, 0); LCD_dat(0x20);
    LCD_goto(8, 1); LCD_dat(0x20);
  }
  lcd_bigchar(0, h / 10); lcd_bigchar(4, h % 10); lcd_bigchar(9, m / 10); lcd_bigchar(13, m % 10);
}
//=============================================================================
void set_freq(uint16_t freq)
{
  freqs = freq;
}
//=============================================================================
uint16_t get_freq(void)
{
  return freqs;
}
//=============================================================================
void show_bigpressure(void)
{
  uint8_t display[4];
  int16_t p;
  if (bmp180HaveSensor())  {
    bmp180Convert();
    p = bmp180GetPressure();
    display[0] = (p / 1000) % 10;
    display[1] = (p / 100) % 10;
    display[2] = (p / 10) % 10;
    display[3] = (p / 1) % 10;
    lcd_bigchar(0, display[0]);
    lcd_bigchar(4, display[1]);
    lcd_bigchar(8, display[2]);
    LCD_goto(13, 0); LCD_puts("MM");
    LCD_goto(13, 1); LCD_puts("RS");
    _delay_ms(2000);
  }
}
//=============================================================================
void show_himedity(void)
{
  uint8_t display[4];
  int16_t p;
  if (dht22HaveSensor()) {
  	p = dht22GetHumidity();
    display[0] = (p / 1000) % 10;
    display[1] = (p / 100) % 10;
    display[2] = (p / 10) % 10;
    display[3] = (p / 1) % 10;
    lcd_bigchar(0, display[1]);
    lcd_bigchar(4, display[2]);
    LCD_dat(4);
    lcd_bigchar(9, display[3]);
    LCD_goto(14, 0); LCD_puts("HM");
    LCD_goto(14, 1); LCD_puts("%%");
    _delay_ms(2000);
  }
}
//=============================================================================
void show_bigfreq(void)
{
  uint8_t display[4];
  display[0] = (freqs / 10000) % 10;
  display[1] = (freqs / 1000) % 10;
  display[2] = (freqs / 100) % 10;
  display[3] = (freqs / 10) % 10;
  if (freqs >= 10000) {
    lcd_bigchar(0, display[0]);
  }
  lcd_bigchar(4, display[1]);
  lcd_bigchar(8, display[2]);
  LCD_dat(4);
  lcd_bigchar(13, display[3]);
  _delay_ms(1000);
  LCD_goto(0, 0); print_dec(rda5807GetChan(), 3, ' ');
  _delay_ms(1000);
}
//=============================================================================
void show_bigtemp(void)
{
  static uint8_t cnt_bmp180 = 0;
  uint8_t display[3];
  int16_t t = ds18x20GetTemp(0);
  if (t < 0) {
    LCD_goto(1, 0); LCD_dat(1); LCD_dat(1);
  }
  display[0] = (t / 100) % 10;
  display[1] = (t / 10) % 10;
  display[2] = (t / 1) % 10;
  lcd_bigchar(4, display[0]);
  lcd_bigchar(8, display[1]);
  LCD_dat(4);
  lcd_bigchar(13, display[2]);
  _delay_ms(1000);
  ds18x20Process();
  cnt_bmp180++;
  if (cnt_bmp180 == 5) {
    cnt_bmp180 = 0;
    if (bmp180HaveSensor()) {
	  bmp180Convert();
      dht22Read();
      RTOS_setTask(EVENT_SENSOR_PRESSURE, 0, 0);
    }
  }
}
//=============================================================================
void RC5_scan(void)
{
  uint16_t command;
  if (RC5_NewCommandReceived(&command))
  {
    RTOS_setTask(RC5_GetCommandBits(command), 0, 0); 
    RC5_Reset();
  }
}
//=============================================================================
void load_eeprom(void)
{
}
//=============================================================================
void save_eeprom(void)
{
}
//=============================================================================
void set_blink(void)
{
  blinks = !blinks;
  RTOS_setTask(EVENT_TIMER_SECOND, 0, 0);
}
//=============================================================================
void DS18x20_scan(void)
{
  RTOS_setTask(EVENT_SENSOR_TEMP, 0, 0);
}
//=============================================================================
void run_main(unsigned char event)
{
  uint8_t vol;
  switch(event) {
    case EVENT_TIMER_SECOND:
      show_bigfreq();
//	  show_bigtime();
    break;
    case EVENT_KEY_LEFT:
	  if (regim == 0) {
	    vol = rda5807GetVolume();
	    rda5807SetVolume(vol - 1);
	  } else {
	    rda5807SetFreq(10120,0);
		set_freq(10120);
	  }
    break;
    case EVENT_KEY_RIGHT:
	  if (regim == 0) {
	    vol = rda5807GetVolume();
	    rda5807SetVolume(vol + 1);
	  } else {
	    rda5807SetFreq(10000,0);
		set_freq(10000);
	  }
    break;
    case EVENT_KEY_SET_LONG:
	  mute = !mute;
      rda5807SetMute(mute);
    break;
    case EVENT_KEY_SET:
	  regim = !regim;
    break;
    case EVENT_SENSOR_TEMP:
	  LCD_clear();
      show_bigtemp();
    break;
    case EVENT_SENSOR_HIMUDATE:
    break;
    case EVENT_SENSOR_PRESSURE:
	  LCD_clear();
      show_bigpressure();
	  LCD_clear();
      show_himedity(); 
    break;
    case EVENT_SET_STATE_OPTION:
	  LCD_clear();
	  SET_STATE(run_option);
  	  lcd_option();
    break;
	default:
    break;
  }
}
//=============================================================================
void run_option(unsigned char event)
{
  switch(event) {
    case EVENT_TIMER_SECOND:
	  lcd_option();
    break;
    case EVENT_KEY_LEFT:
      BEEP_beep();
      if (o_menu > 0) { o_menu--; } else { o_menu = OMENU_MAX; }
	  lcd_option();
    break;
    case EVENT_KEY_RIGHT:
      BEEP_beep();
      if (o_menu < OMENU_MAX) { o_menu++; } else { o_menu = 0; }
	  lcd_option();
    break;
    case EVENT_KEY_SET_LONG:
	  LCD_clear();
	  SET_STATE(run_main);
      RTOS_setTask(EVENT_TIMER_SECOND, 0, 0);
    break;
  }
}
//=============================================================================
void lcd_option(void)
{
  unsigned char h, m, s;
  unsigned char d, ms, y, dw;
  LCD_goto(0, 0);
  LCD_puts(optionmenu[o_menu]);
  if (o_menu == OM_SETTIME) {
    RTC_get_time(&h, &m, &s);
    LCD_goto(0, 1);
    print_dec(h,2,'0'); LCD_dat(':'); print_dec(m,2,'0'); LCD_dat(':'); print_dec(s,2,'0'); 
    LCD_puts("          ");
  } else if (o_menu == OM_SETDATE) {
    RTC_get_date(&dw, &d, &ms, &y);
    dw = RTC_day_of_week(d, ms, y);
    LCD_goto(0, 1);
    print_dec(d,2,'0'); LCD_dat('-'); print_dec(ms,2,'0'); LCD_dat('-'); print_dec(2000 + y,4,'0'); 
    LCD_dat(' '); LCD_dat('['); LCD_puts(den_dw[dw]); LCD_dat(']'); LCD_dat(' ');
  } else if (o_menu == OM_SETALARM) {
    LCD_goto(0, 1);
    print_dec(a_hour,2,'0'); LCD_dat(':'); print_dec(a_min,2,'0'); LCD_dat(':'); print_dec(a_sec,2,'0');
    LCD_dat(' '); LCD_dat('-'); LCD_dat(' '); LCD_dat('[');
    if (a_onoff == 1) LCD_puts(" ON"); else LCD_puts("OFF");
	LCD_dat(']');
  }                                                            
}
//=============================================================================
