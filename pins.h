#ifndef PINS_H
#define PINS_H
//=============================================================================
#include <avr/io.h>
//=============================================================================
#define CONCAT(x,y)			x ## y

#define DDR(x)				CONCAT(DDR,x)
#define PORT(x)				CONCAT(PORT,x)
#define PIN(x)				CONCAT(PIN,x)

#define I2C_SCL				D
#define I2C_SCL_LINE		(1<<6)
#define I2C_SDA				D
#define I2C_SDA_LINE		(1<<7)

// One ware interface
#define ONE_WIRE			D
#define ONE_WIRE_LINE		(1<<5)
#define DHT_DATA			B
#define DHT_DATA_LINE		(1<<0)

#define PORT_LIGHT			B
#define PORT_LIGHT_LINE		(1<<1)

// Beeper
#define BEEPER				D
#define BEEPER_LINE			(1<<4)

#define BUTTON_1			D
#define BUTTON_1_LINE		(1<<0)

// Keyboard
#define BUTTON_SET			D
#define BUTTON_SET_LINE		0
//=============================================================================
#endif /* PINS_H */
