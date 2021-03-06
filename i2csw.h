#ifndef I2CSW_H
#define I2CSW_H

#include <inttypes.h>
#include "pins.h"

#define I2C_NOACK	0
#define I2C_ACK		1
#define I2C_READ	1

void I2CswStart(uint8_t addr);
void I2CswStop(void);

void I2CswWriteByte(uint8_t data);
uint8_t I2CswReadByte(uint8_t ack);

#endif /* I2CSW_H */
