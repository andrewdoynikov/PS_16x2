#ifndef UTILS_H
#define UTILS_H	          
//=============================================================================
#include <avr/io.h>
//=============================================================================
#define BEEP_DDR			DDRD
#define BEEP_PORT			PORTD
#define BEEP_PIN			4	
#define BEEP_TIME			30
#define BEEP_LONG_TIME		200
//=============================================================================
void BEEP_init(void);
void BEEP_beep(void);
void BEEP_long(void);
void print_dec(unsigned int x, unsigned char n, unsigned char fillch);
//=============================================================================
#endif
