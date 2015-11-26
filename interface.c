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
#define OMENU_MAX 3
char *optionmenu[] = {
"Set TIME        ", 
"Set DATE        ", 
"Set ALARM       ", 
"Set FM STATIONS " 
};
#define OM_SETTIME     	0
#define OM_SETDATE     	1
#define OM_SETALARM    	2
#define OM_SETFM    	3
uint8_t o_menu = OM_SETTIME;
//=============================================================================
#define DEBUGER		0

#define SHOW_FREQ					3000 // ms
#define SHOW_TIME				      30 // seconds
#define SHOW_TEMP					5000 // ms
#define SHOW_PRESURE				5000 // ms
#define SHOW_HIMEDITY				5000 // ms
#define SHOW_CHANEL					3000 // ms
#define SHOW_VOLUME					3000 // ms
//=============================================================================
unsigned char a_onoff, a_hour, a_min, a_sec;
unsigned char n_edit_time = 0, n_edit_date = 0, n_edit_alarm = 0;
unsigned char hour, min, sec;
unsigned char wday, day, mes, year;
//=============================================================================
//                   {0,  1,   2,   3,   4,   5,   6,   7,   8,    9,    10,   11,   12,   13,   14,   15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25};
uint8_t stations[] = {70, 83,  90,  94,  98,  117, 121, 125, 130,  134,  138,  142,  146,  150,  154,  159,  165,  171,  175,  179,  184,  189,  193,  198,  203,  208};
//                   9400 9530 9600 9640 9680 9870 9910 9950 10000 10040 10080 10120 10160 10200 10240 10290 10350 10410 10450 10490 10540 10590 10630 10680 10730 10780
//=============================================================================
extern void (*pState)(unsigned char event);
//=============================================================================
#define SET_STATE(a) pState = a  // макрос для смены состояния
unsigned char blinks = 0;
unsigned char sensor_num = 0;
static unsigned char mute = 1, chanel = 11;
static uint16_t cnt_sensor = 0;
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
  }
}
//=============================================================================
void show_himedity(void)
{
  uint8_t display[4];
  int16_t p;
  dht22Read();
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
  }
}
//=============================================================================
void show_bigvolume(void)
{
  uint8_t display[2];
  uint8_t v = rda5807GetVolume();
  display[0] = (v / 10) % 10;
  display[1] = (v / 1) % 10;
  LCD_goto(0, 0); LCD_puts("VOLUME:");
  lcd_bigchar(8, display[0]);
  lcd_bigchar(12, display[1]);
}
//=============================================================================
void show_bigchanel(void)
{
  uint8_t display[2];
  uint8_t v = chanel + 1;
  display[0] = (v / 10) % 10;
  display[1] = (v / 1) % 10;
  LCD_goto(0, 0); LCD_puts("CHANEL:");
  lcd_bigchar(8, display[0]);
  lcd_bigchar(12, display[1]);
}
//=============================================================================
void show_bigfreq(void)
{
  uint8_t display[4];
  uint16_t f = rda5807GetFreq();
  display[0] = (f / 10000) % 10;
  display[1] = (f / 1000) % 10;
  display[2] = (f / 100) % 10;
  display[3] = (f / 10) % 10;
  if (f >= 10000) {
    lcd_bigchar(0, display[0]);
  }
  lcd_bigchar(4, display[1]);
  lcd_bigchar(8, display[2]);
  LCD_dat(4);
  lcd_bigchar(13, display[3]);
}
//=============================================================================
void show_bigtemp(void)
{
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
  ds18x20Process();
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
void run_main(unsigned char event)
{
  switch(event) {
    case EVENT_TIMER_SECOND:
	  if (cnt_sensor > 0) {
	    cnt_sensor--;
	  }
    break;
    case EVENT_SET_SHOW:
	  LCD_clear();
      RTOS_setTask(EVENT_SHOW_FREQ, 0, 0);
    break;
    case EVENT_SHOW_FREQ:
	  cnt_sensor = SHOW_TIME;
	  if (mute == 0) {
	    LCD_clear();
        show_bigfreq();
        RTOS_setTask(EVENT_SHOW_TIME, SHOW_FREQ, 0);
	  } else {
        RTOS_setTask(EVENT_SHOW_TIME, 0, 0);
	  }
    break;
    case EVENT_SHOW_TIME:
	  if (cnt_sensor > 0) {
	    show_bigtime();
        RTOS_setTask(EVENT_SHOW_TIME, 1000, 0);
	  } else {
        RTOS_setTask(EVENT_SENSOR_TEMP, 0, 0);
	  }
    break;
    case EVENT_SENSOR_TEMP:
	  LCD_clear();
      show_bigtemp();
      RTOS_setTask(EVENT_SENSOR_HIMUDATE, SHOW_TEMP, 0);
    break;
    case EVENT_SENSOR_HIMUDATE:
	  LCD_clear();
      show_himedity(); 
      RTOS_setTask(EVENT_SENSOR_PRESSURE, SHOW_HIMEDITY, 0);
    break;
    case EVENT_SENSOR_PRESSURE:
	  LCD_clear();
      show_bigpressure();
      RTOS_setTask(EVENT_SHOW_FREQ, SHOW_PRESURE, 0);
    break;
    case EVENT_RC5_UP:
      RTOS_setTask(EVENT_KEY_RIGHT, 0, 0);
    break;
    case EVENT_RC5_DOWN:
      RTOS_setTask(EVENT_KEY_LEFT, 0, 0);
    break;
    case EVENT_RC5_LEFT:
      RTOS_setTask(EVENT_KEY_LEFT, 0, 0);
    break;
    case EVENT_RC5_RIGHT:
      RTOS_setTask(EVENT_KEY_RIGHT, 0, 0);
    break;
    case EVENT_KEY_LEFT:
	  clear_task();
	  if (mute == 0) {
        rda5807SetVolume(rda5807GetVolume() - 1);
        RTOS_setTask(EVENT_SHOW_VOLUME, 0, 0);
	  }
    break;
    case EVENT_KEY_RIGHT:
	  clear_task();
	  if (mute == 0) {
        rda5807SetVolume(rda5807GetVolume() + 1);
        RTOS_setTask(EVENT_SHOW_VOLUME, 0, 0);
	  }
    break;
    case EVENT_RC5_KEY1:
    case EVENT_RC5_KEY2:
    case EVENT_RC5_KEY3:
    case EVENT_RC5_KEY4:
    case EVENT_RC5_KEY5:
    case EVENT_RC5_KEY6:
    case EVENT_RC5_KEY7:
    case EVENT_RC5_KEY8:
    case EVENT_RC5_KEY9:
    break;
    case EVENT_KEY_SET_LONG:
    break;
    case EVENT_RC5_SET:
    case EVENT_KEY_SET:
	  mute = !mute;
	  if (mute == 0) {
	    clear_task();
	    cnt_sensor = 0;
	    rda5807SetMute(0);
        rda5807SetVolume(rda5807GetVolume());
		// get chanel save eeprom ????????????????
        rda5807SetChan(stations[chanel], 0);
        RTOS_setTask(EVENT_SHOW_CHANEL, 0, 0);
        RTOS_setTask(EVENT_SAVE_CHANEL, 3000, 0);
        RTOS_setTask(EVENT_SAVE_VOLUME, 3000, 0);
	  } else {
	    rda5807SetMute(1);
	  }
    break;
    case EVENT_KEY_SET_DOUBLE:
    break;
    case EVENT_SET_STATE_OPTION:
	  LCD_clear();
	  SET_STATE(run_option);
  	  lcd_option();
    break;
    case EVENT_SHOW_CHANEL:
	  cnt_sensor = 0;
	  LCD_clear();
	  show_bigchanel();
      RTOS_setTask(EVENT_SHOW_FREQ, SHOW_CHANEL, 0);
    break;
    case EVENT_SHOW_VOLUME:
	  cnt_sensor = 0;
	  LCD_clear();
	  show_bigvolume();
      RTOS_setTask(EVENT_SHOW_FREQ, SHOW_VOLUME, 0);
    break;
    case EVENT_SAVE_CHANEL:
      BEEP_beep();
    break;
    case EVENT_SAVE_VOLUME:
      BEEP_beep();
    break;
	default:
    break;
  }
}
//=============================================================================
void clear_task(void)
{
  RTOS_deleteTask(EVENT_SHOW_FREQ);
  RTOS_deleteTask(EVENT_SHOW_TIME);
  RTOS_deleteTask(EVENT_SENSOR_HIMUDATE);
  RTOS_deleteTask(EVENT_SENSOR_PRESSURE);
  RTOS_deleteTask(EVENT_SENSOR_TEMP);
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
    case EVENT_KEY_SET:
	  if (o_menu == OM_SETTIME) {
        n_edit_time = 0;
        RTC_get_time(&hour, &min, &sec);
        SET_STATE(edit_time);
        lcd_edit_time();
	  } else if (o_menu == OM_SETDATE) {
 	    n_edit_date = 0;
        RTC_get_date(&wday, &day, &mes, &year);
		wday = RTC_day_of_week(day, mes, year);
        SET_STATE(edit_date);
        lcd_edit_date();
	  } else if (o_menu == OM_SETALARM) {
 	    n_edit_alarm = 0;
//        RTC_get_alarm(&a_hour, &a_min, &a_sec);
        SET_STATE(edit_alarm);
        lcd_edit_alarm();
	  } else if (o_menu == OM_SETFM) {
        SET_STATE(edit_fmstation);
        lcd_edit_fmstation();
      }
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
  } else if (o_menu == OM_SETFM) {
  }
}
//=============================================================================
void edit_time(unsigned char event)
{
  switch(event) {
    case EVENT_KEY_SET_HOLD:
    break;
    case EVENT_KEY_SET_LONG:
    break;
    case EVENT_KEY_SET:
      BEEP_beep();
	  n_edit_time++;
	  if (n_edit_time > 1) {
	    n_edit_time = 0;
		RTC_set_time(hour, min, 0);
        SET_STATE(run_option);
		lcd_option();
		return;
	  }
      lcd_edit_time();
    break;
    case EVENT_KEY_LEFT:
      BEEP_beep();
	  if (n_edit_time == 0) { if (hour > 0) hour--; else hour = 23; } 
	  if (n_edit_time == 1) { if (min > 0) min--; else min = 59; }
      lcd_edit_time();
    break;
    case EVENT_KEY_RIGHT:
      BEEP_beep();
	  if (n_edit_time == 0) { if (hour < 23) hour++;  else hour = 0; } 
	  if (n_edit_time == 1) { if (min < 59) min++; else min = 0; }
      lcd_edit_time();
    break;
    case EVENT_TIMER_SECOND:
	  lcd_edit_time();
    break;
  }    
}
//=============================================================================
void lcd_edit_time(void)
{
  LCD_goto(0, 0);
  LCD_puts(optionmenu[o_menu]);
  if (n_edit_time == 0)
  {
    LCD_goto(9, 0);
    LCD_puts(" [HOUR]");
    LCD_goto(0, 1);
	if (blinks == 1) print_dec(hour, 2, '0'); else { LCD_dat(' '); LCD_dat(' '); }
    LCD_dat(':'); print_dec(min, 2, '0'); LCD_dat(':'); print_dec(0, 2, '0');
  } else if (n_edit_time == 1) {
    LCD_goto(9, 0);
    LCD_puts(" [MIN] ");
    LCD_goto(0, 1); print_dec(hour, 2, '0'); LCD_dat(':');
	if (blinks == 1) print_dec(min, 2, '0'); else { LCD_dat(' '); LCD_dat(' '); }
    LCD_dat(':'); print_dec(0, 2, '0');
  }
}
//=============================================================================
void edit_date(unsigned char event)
{
  switch(event) {
    case EVENT_KEY_SET_HOLD:
    break;
    case EVENT_KEY_SET_LONG:
    break;
    case EVENT_KEY_SET:
      BEEP_beep();
	  n_edit_date++;
	  wday = RTC_day_of_week(day, mes, year);
	  if (n_edit_date > 2) {
	    n_edit_date = 0;
		RTC_set_date(wday, day, mes, year);
        SET_STATE(run_option);
		lcd_option();
		return;
	  }
      lcd_edit_date();
    break;
    case EVENT_KEY_LEFT:
      BEEP_beep();
	  if (n_edit_date == 0)
	  {
	    if (day > 1) { day--; } else { day = 31; }
	  } else if (n_edit_date == 1) {
	    if (mes > 1) { mes--; } else { mes = 12; }
	  } else if (n_edit_date == 2) {
	    if (year > 15) { year--; } else { year = 15; }
      }
	  wday = RTC_day_of_week(day, mes, year);
      lcd_edit_date();
    break;
    case EVENT_KEY_RIGHT:
      BEEP_beep();
	  if (n_edit_date == 0)
	  {
	    if (day < 31) { day++; } else { day = 1; }
	  } else if (n_edit_date == 1) {
	    if (mes < 12) { mes++; } else { mes = 1; }
	  } else if (n_edit_date == 2) { year++; }
	  wday = RTC_day_of_week(day, mes, year);
      lcd_edit_date();
    break;
    case EVENT_TIMER_SECOND:
	  lcd_edit_date();
    break;
  }    
}
//=============================================================================
void lcd_edit_date(void)
{
  LCD_goto(0, 0);
  LCD_puts(optionmenu[o_menu]);
  if (n_edit_date == 0)
  {
    LCD_goto(9, 0);
    LCD_puts(" [DAY]");
    LCD_goto(0, 1);
	if (blinks == 1) print_dec(day, 2, '0'); else { LCD_dat(' '); LCD_dat(' '); }
    LCD_dat('-');
    print_dec(mes, 2, '0');
    LCD_dat('-');
    print_dec(2000 + year, 4, '0');
    LCD_dat(' '); LCD_dat('['); LCD_puts(den_dw[wday]); LCD_dat(']');
  } else if (n_edit_date == 1) {
    LCD_goto(9, 0);
    LCD_puts(" [MES] ");
    LCD_goto(0, 1);
    print_dec(day, 2, '0');
    LCD_dat('-');
	if (blinks == 1) print_dec(mes, 2, '0'); else { LCD_dat(' '); LCD_dat(' '); }
    LCD_dat('-');
    print_dec(2000 + year, 4, '0');
    LCD_dat(' '); LCD_dat('['); LCD_puts(den_dw[wday]); LCD_dat(']');
  } else if (n_edit_date == 2) {
    LCD_goto(9, 0);
    LCD_puts(" [YEAH]");
    LCD_goto(0, 1);
    print_dec(day, 2, '0');
    LCD_dat('-');
    print_dec(mes, 2, '0');
    LCD_dat('-');
	if (blinks == 1) { print_dec(2000 + year, 4, '0'); } else { LCD_puts("    "); }
    LCD_dat(' '); LCD_dat('['); LCD_puts(den_dw[wday]); LCD_dat(']');
  }
}
//=============================================================================
void edit_alarm(unsigned char event)
{
  switch(event) {
    case EVENT_KEY_SET_HOLD:
    break;
    case EVENT_KEY_SET_LONG:
    break;
    case EVENT_KEY_SET:
      BEEP_beep();
      if (a_onoff == 1) n_edit_alarm++; else n_edit_alarm = 3;
      if (n_edit_alarm > 2) {
	    n_edit_alarm = 0;
//		RTC_set_alarm(a_hour, a_min, 0);
//		save_alarm_mod(a_onoff);
        SET_STATE(run_option);
	  }
      lcd_edit_alarm();
    break;
    case EVENT_KEY_LEFT:
      BEEP_beep();
	  if (n_edit_alarm == 1)
	  {
	    if (a_hour > 0) a_hour--; else { a_hour = 23; }
	  } 
	  if (n_edit_alarm == 2) {
	    if (a_min > 0) a_min--; else { a_min = 59; }
	  }
	  if (n_edit_alarm == 0) {
	    a_onoff = !a_onoff;
	  }
      lcd_edit_alarm();
    break;
    case EVENT_KEY_RIGHT:
      BEEP_beep();
	  if (n_edit_alarm == 1)
	  {
	    if (a_hour < 23) a_hour++; else { a_hour = 0; }
	  } 
	  if (n_edit_alarm == 2) {
	    if (a_min < 59) a_min++; else { a_min = 0; }
	  }
	  if (n_edit_alarm == 0) {
	    a_onoff = !a_onoff;
	  }
      lcd_edit_alarm();
    break;
    case EVENT_TIMER_SECOND:
	  lcd_edit_alarm();
    break;
  }    
}
//=============================================================================
void lcd_edit_alarm(void)
{
  LCD_goto(0, 0);
  LCD_puts(optionmenu[o_menu]);
  if (n_edit_alarm == 1)
  {
    LCD_goto(9, 0);
    LCD_puts(" [HOUR]");
    LCD_goto(0, 1);
	if (blinks == 1) print_dec(a_hour, 2, '0'); else { LCD_dat(' '); LCD_dat(' '); }
    LCD_dat(':');
    print_dec(a_min, 2, '0');
    LCD_dat(':');
    LCD_dat('0'); LCD_dat('0');
    LCD_dat(' '); LCD_dat('-'); LCD_dat(' '); LCD_dat('[');
    if (a_onoff == 1) LCD_puts(" ON"); else LCD_puts("OFF");
    LCD_dat(']');
  } else if (n_edit_alarm == 2) {
    LCD_goto(9, 0);
    LCD_puts(" [MIN] ");
    LCD_goto(0, 1);
    print_dec(a_hour, 2, '0');
    LCD_dat(':');
	if (blinks == 1) print_dec(a_min, 2, '0'); else { LCD_dat(' '); LCD_dat(' '); }
    LCD_dat(':');
    LCD_dat('0'); LCD_dat('0');
    LCD_dat(' '); LCD_dat('-'); LCD_dat(' '); LCD_dat('[');
    if (a_onoff == 1) LCD_puts(" ON"); else LCD_puts("OFF");
    LCD_dat(']');
  } else if (n_edit_alarm == 0) {
    LCD_goto(9, 0);
    LCD_puts(" [MOD] ");
    LCD_goto(0, 1);
    print_dec(a_hour, 2, '0');
    LCD_dat(':');
    print_dec(a_min, 2, '0');
    LCD_dat(':');
    LCD_dat('0'); LCD_dat('0');
    LCD_dat(' '); LCD_dat('-'); LCD_dat(' '); LCD_dat('[');
    if (blinks == 1) {
      if (a_onoff == 1) LCD_puts(" ON"); else LCD_puts("OFF");
    } else {
      LCD_puts("   ");
    }
    LCD_dat(']');
  }
}
//=============================================================================
void edit_fmstation(unsigned char event)
{
}
//=============================================================================
void lcd_edit_fmstation(void)
{
}
//=============================================================================
