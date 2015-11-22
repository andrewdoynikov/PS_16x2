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
#define SHOW_TIME_SENSOR_NUM		1
#define SHOW_TIME_SENSOR_TEMP		5
#define SHOW_TIME_SENSOR_PRESURE	50
#define SHOW_TIME_SENSOR_HIMEDITY	100

#define SHOW_FREQ					30
#define SHOW_TIME					60
#define SHOW_TEMP					5
#define SHOW_PRESURE				5
#define SHOW_HIMEDITY				5

#define MODE_FREQ					0
#define MODE_TIME					1
#define MODE_TEMP					2
#define MODE_PRESURE				3
#define MODE_HIMEDITY				4
#define MODE_VOLUME					5
//=============================================================================
unsigned char a_onoff, a_hour, a_min, a_sec;
unsigned char n_edit_time = 0, n_edit_date = 0, n_edit_alarm = 0;
unsigned char hour, min, sec;
unsigned char wday, day, mes, year;
//=============================================================================
uint8_t stations[] = {70,83,90,94,98,117,121,125,130,134,138,142,146,150,154,159,165,171,175,179,184,189,193,198,203,208};
//=============================================================================
extern void (*pState)(unsigned char event);
//=============================================================================
#define SET_STATE(a) pState = a  // макрос для смены состояния
unsigned char blinks = 0;
unsigned char sensor_num = 0;
unsigned char mute = 1, mode = 0, fm_mode = 0, chanel = 0;
unsigned char regim = 0;
static unsigned char cnt_sensor = 0, cnt_freq = 0;
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

void show_bigfreq(void)
{
  uint8_t display[4];
  uint16_t freqs = rda5807GetFreq();
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
  cnt_sensor++;
  if (cnt_freq > 0) {
    if (cnt_freq == SHOW_FREQ) {
      RTOS_setTask(EVENT_SHOW_FREQ, 0, 0);
	  mode = MODE_FREQ;
	}
    cnt_sensor = 0;
    cnt_freq--;
  } else if (cnt_sensor < SHOW_TIME) {
    RTOS_setTask(EVENT_TIMER_SECOND, 0, 0);
    mode = MODE_TIME;
  } else if (cnt_sensor == SHOW_TIME) {
    RTOS_setTask(EVENT_SENSOR_TEMP, 0, 0);
    mode = MODE_TEMP;
  } else if (cnt_sensor == SHOW_TIME + SHOW_TEMP) {
    RTOS_setTask(EVENT_SENSOR_PRESSURE, 0, 0);
    mode = MODE_PRESURE;
  } else if (cnt_sensor == SHOW_TIME + SHOW_TEMP + SHOW_PRESURE) {
    RTOS_setTask(EVENT_SENSOR_HIMUDATE, 0, 0);
    mode = MODE_HIMEDITY;
  } else if (cnt_sensor == SHOW_TIME + SHOW_TEMP + SHOW_PRESURE + SHOW_HIMEDITY) {
    cnt_sensor = 0;
	if (mute == 0) {
      cnt_freq = SHOW_FREQ;
	  mode = MODE_FREQ;
	} else { 
	  cnt_freq = 0;
      RTOS_setTask(EVENT_TIMER_SECOND, 0, 0);
      mode = MODE_TIME;
    }
  }
}
//=============================================================================
void run_main(unsigned char event)
{
  switch(event) {
    case EVENT_TIMER_SECOND:
	  show_bigtime();
    break;
    case EVENT_KEY_LEFT:
	  if (fm_mode == 0) {
	    if (mute == 0) {
          rda5807SetVolume(rda5807GetVolume() - 1);
          RTOS_setTask(EVENT_SHOW_VOLUME, 0, 0);
	    }
	  } else {
	    if (chanel > 0) chanel--;
        rda5807SetChan(stations[chanel], 0);
        cnt_sensor = 0;
        cnt_freq = SHOW_FREQ;
	    mode = MODE_FREQ;
	  }
    break;
    case EVENT_KEY_RIGHT:
	  if (fm_mode == 0) {
	    if (mute == 0) {
          rda5807SetVolume(rda5807GetVolume() + 1);
          RTOS_setTask(EVENT_SHOW_VOLUME, 0, 0);
	    }
	  } else {
	    if (chanel < 25) chanel++;
        rda5807SetChan(stations[chanel], 0);
        cnt_sensor = 0;
        cnt_freq = SHOW_FREQ;
	    mode = MODE_FREQ;
	  }
    break;
    case EVENT_SHOW_VOLUME:
	  if (mode != MODE_VOLUME) LCD_clear();
      mode = MODE_VOLUME;
      show_bigvolume();
      RTOS_setTask(EVENT_STOP_SHOW_VOLUME, 2000, 0);
    break;
    case EVENT_STOP_SHOW_VOLUME:
      cnt_freq = SHOW_FREQ;
      cnt_sensor = 0;
      mode = MODE_FREQ;
    break;
    case EVENT_KEY_SET_LONG:
      fm_mode = !fm_mode;
    break;
    case EVENT_KEY_SET:
    break;
    case EVENT_KEY_SET_DOUBLE:
	  mute = !mute;
	  if (mute == 1) {
	    cnt_freq = 0;
	    rda5807SetMute(1);
	  } else {
        rda5807SetChan(stations[chanel], 0);
	    rda5807SetMute(0);
        rda5807SetVolume(10);
        cnt_freq = SHOW_FREQ;
	    mode = MODE_FREQ;
	  }
    break;
    case EVENT_SHOW_FREQ:
	  LCD_clear();
      show_bigfreq();
    break;
    case EVENT_SENSOR_TEMP:
	  LCD_clear();
      show_bigtemp();
    break;
    case EVENT_SENSOR_HIMUDATE:
	  LCD_clear();
      show_himedity(); 
    break;
    case EVENT_SENSOR_PRESSURE:
	  LCD_clear();
      show_bigpressure();
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
