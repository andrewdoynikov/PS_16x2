#include <avr/io.h>
#include "ds3231.h"
#include "i2csw.h"
//=============================================================================
uint8_t dec2bcd(uint8_t d)
{
  return ((d / 10 * 16) + (d % 10));
}
//=============================================================================
uint8_t bcd2dec(uint8_t b)
{
  return ((b / 16 * 10) + (b % 16));
}
//=============================================================================
uint8_t bcd2bin(uint8_t b)
{
  return ((b / 16 * 10) + (b % 16));
}
//=============================================================================
// Чтение байта относительно 0 регистра
//=============================================================================
uint8_t RTC_read_byte(uint8_t offset)
{
  uint8_t ret;
  I2CswStart(DS_SLAVE_ADDR_W);
  I2CswWriteByte(offset);
  I2CswStop();
  I2CswStart(DS_SLAVE_ADDR_R);
  ret = I2CswReadByte(0);
  I2CswStop();
  return ret;
}

//=============================================================================
// Запись байта относительно 0 регистра
//=============================================================================
void RTC_write_byte(uint8_t b, uint8_t offset)
{
  I2CswStart(DS_SLAVE_ADDR_W);
  I2CswWriteByte(offset);
  I2CswWriteByte(b);
  I2CswStop();
}

//=============================================================================
// Инициализация модуля RTC DS3231
//=============================================================================
void RTC_init(void)
{
  I2CswStart(DS_SLAVE_ADDR_W);
  I2CswWriteByte(0x0E);
  I2CswWriteByte(0x20);
  I2CswWriteByte(0x08);
  I2CswStop();
}

//=============================================================================
// Чтение времени
//=============================================================================
void RTC_get_time(uint8_t* hour, uint8_t* min, uint8_t* sec)
{
  I2CswStart(DS_SLAVE_ADDR_W);
  I2CswWriteByte(0x00);
  I2CswStop();
  I2CswStart(DS_SLAVE_ADDR_R);
  *sec = bcd2dec(I2CswReadByte(1)); 
  *min = bcd2dec(I2CswReadByte(1)); 
  *hour = bcd2dec(I2CswReadByte(0)); 
  I2CswStop();
}

//=============================================================================
// Установка времени
//=============================================================================
void RTC_set_time(uint8_t hour, uint8_t min, uint8_t sec)
{
  I2CswStart(DS_SLAVE_ADDR_W);
  I2CswWriteByte(0x00);
  I2CswWriteByte(dec2bcd(sec)); // seconds
  I2CswWriteByte(dec2bcd(min)); // minutes
  I2CswWriteByte(dec2bcd(hour)); // hours
  I2CswStop();
}

//=============================================================================
// Чтение даты
//=============================================================================
void RTC_get_date(uint8_t* wday, uint8_t* day, uint8_t* mes, uint8_t* year)
{
  I2CswStart(DS_SLAVE_ADDR_W);
  I2CswWriteByte(0x03);
  I2CswStop();
  I2CswStart(DS_SLAVE_ADDR_R);
  *wday = bcd2dec(I2CswReadByte(1)); 
  *day = bcd2dec(I2CswReadByte(1)); 
  *mes = bcd2dec(I2CswReadByte(1)); 
  *year = bcd2dec(I2CswReadByte(0)); 
  I2CswStop();
}

//=============================================================================
// Установка даты
//=============================================================================
void RTC_set_date(uint8_t wday, uint8_t day, uint8_t mes, uint8_t year)
{
  uint8_t dw = RTC_day_of_week(day, mes, year);
  I2CswStart(DS_SLAVE_ADDR_W);
  I2CswWriteByte(0x03);
  I2CswWriteByte(dec2bcd(dw + 1));
  I2CswWriteByte(dec2bcd(day));
  I2CswWriteByte(dec2bcd(mes));
  I2CswWriteByte(dec2bcd(year));
  I2CswStop();
}

//=============================================================================
uint8_t RTC_day_of_week(uint8_t day, uint8_t month, uint8_t year)
{
  if (month < 3) {
    month += 12;
    year--;
  }
  return (((13 * month + 3) / 5 + day + year + year / 4 - year / 100 + year / 400) % 7);
}
//=============================================================================
