#ifndef INTERFACE_H
#define INTERFACE_H
//=============================================================================
#define TEMP_MEASURE_TIME		1000
#define SENSOR_POLL_INTERVAL	5000
//=============================================================================
void lcd_bigchar(unsigned char col, unsigned char x);
void run_main(unsigned char event);
void run_option(unsigned char event);
void lcd_option(void);
void set_blink(void);
void RC5_scan(void);
void show_bigvolume(void);
void show_bigtime(void);
void show_bigfreq(void);
void show_bigtemp(void);
void show_bigpressure(void);
void show_himedity(void);
void show_bigchanel(void);
void load_eeprom(void);
void save_eeprom(void);
//=============================================================================
void edit_time(unsigned char event);
void lcd_edit_time(void);
void edit_date(unsigned char event);
void lcd_edit_date(void);
void edit_alarm(unsigned char event);
void lcd_edit_alarm(void);
void edit_fmstation(unsigned char event);
void lcd_edit_fmstation(void);
#endif
