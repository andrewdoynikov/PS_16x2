#include <util/delay.h>
#include "rda5807.h"

#include "i2csw.h"

static uint8_t wrBuf[8];
static uint8_t rdBuf[12];
static uint8_t _volume = RDA5807_VOL_MAX;
static uint16_t freqs = 10000;

static void rda5807WriteI2C(void)
{
	uint8_t i;

	I2CswStart(RDA5807M_I2C_ADDR);
	for (i = 0; i < sizeof(wrBuf); i++)
		I2CswWriteByte(wrBuf[i]);
	I2CswStop();

	return;
}

void rda5807Init(void)
{
	wrBuf[0] = RDA5807_DHIZ;
	wrBuf[1] = RDA5807_CLK_MODE_32768 | RDA5807_RDS_EN | RDA5807_NEW_METHOD | RDA5807_ENABLE;
	wrBuf[2] = 0;
	wrBuf[3] = RDA5807_BAND | RDA5807_SPACE;
	wrBuf[4] = 0;
	wrBuf[5] = 0;
	wrBuf[6] = 0b1000 & RDA5807_SEEKTH;
	wrBuf[7] = RDA5807_LNA_PORT_SEL | RDA5807_VOLUME;
	return;
}

void rda5807SetFreq(uint16_t freq, uint8_t mono)
{
	uint16_t chan = (freq - RDA5807_FREQ_MIN) / RDA5807_CHAN_SPACING;
    freqs = freq;
	rda5807SetChan(chan, mono);

	return;
}

uint16_t rda5807GetFreq(void)
{
  return freqs;
}

uint8_t rda5807GetChan(void)
{
  uint8_t hi, lo;
  I2CswStart(RDA5807M_I2C_ADDR | I2C_READ);
  hi = I2CswReadByte(I2C_ACK);
  lo = I2CswReadByte(I2C_NOACK) ;
  I2CswStop();
  uint16_t c = ((256 * hi) + lo);
  return (c & 0x3FF);
}

void rda5807SetChan(uint8_t chan, uint8_t mono)
{
	if (mono)
		wrBuf[0] |= RDA5807_MONO;
	else
		wrBuf[0] &= ~RDA5807_MONO;

	wrBuf[2] = chan >> 2;								/* 8 MSB */

	wrBuf[3] &= 0x3F;
	wrBuf[3] |= RDA5807_TUNE | ((chan & 0x03) << 6);	/* 2 LSB */

	rda5807WriteI2C();

	return;
}

uint8_t *rda5807ReadStatus(void)
{
	uint8_t i;

	I2CswStart(RDA5807M_I2C_ADDR | I2C_READ);
	for (i = 0; i < sizeof(rdBuf) - 1; i++)
		rdBuf[i] = I2CswReadByte(I2C_ACK);
	rdBuf[sizeof(rdBuf) - 1] = I2CswReadByte(I2C_NOACK);
	I2CswStop();

	/* If seek/tune is complete and current channel is a station */
	if ((rdBuf[0] & RDA5807_STC) && (rdBuf[2] & RDA5807_FM_TRUE)) {
	}

	return rdBuf;
}

void rda5807SetMute(uint8_t mute)
{
	if (mute)
		wrBuf[0] &= ~RDA5807_DMUTE;
	else
		wrBuf[0] |= RDA5807_DMUTE;
	//wrBuf[3] &= ~RDA5807_TUNE;

	rda5807WriteI2C();

	return;
}

uint8_t rda5807GetVolume(void)
{
  return _volume;
}

void rda5807SetVolume(int8_t value)
{
  uint8_t v;
  v = wrBuf[7] & 0xF0;
  if (value <= 15) {
    wrBuf[7] = v + value;
    rda5807WriteI2C();
	_volume = value;
  }
  _delay_ms(50);
  return;
}

void rda5807PowerOn(void)
{
	wrBuf[1] |= RDA5807_ENABLE;
	rda5807WriteI2C();
	return;
}

void rda5807PowerOff(void)
{
	wrBuf[1] &= ~RDA5807_ENABLE;
	rda5807WriteI2C();
	return;
}
