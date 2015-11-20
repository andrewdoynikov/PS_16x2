#ifndef INTERFACE_H
#define INTERFACE_H
//=============================================================================
#define TEMP_MEASURE_TIME		1000
#define SENSOR_POLL_INTERVAL	5000
//=============================================================================
void run_main(unsigned char event);
void run_option(unsigned char event);
void lcd_option(void);
void set_blink(void);
void DS18x20_scan(void);
void RC5_scan(void);
void load_eeprom(void);
void save_eeprom(void);
void lcd_bigchar(unsigned char col, unsigned char x);
void show_bigtime(void);
void show_bigfreq(void);
void show_bigtemp(void);
void show_bigpressure(void);
void show_himedity(void);
void set_freq(uint16_t freq);
uint16_t get_freq(void);
//=============================================================================
#endif
