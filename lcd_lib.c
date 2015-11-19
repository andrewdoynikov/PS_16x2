#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <inttypes.h>
//=============================================================================
#include "lcd_lib.h"
//=============================================================================
const uint8_t LcdCustomChar[] PROGMEM = //define 8 custom LCD chars
{
  0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, // 0. 0/5 full progress block
  0x10, 0x10, 0x10, 0x15, 0x10, 0x10, 0x10, 0x00, // 1. 1/5 full progress block
  0x18, 0x18, 0x18, 0x1D, 0x18, 0x18, 0x18, 0x00, // 2. 2/5 full progress block
  0x1C, 0x1C, 0x1C, 0x1D, 0x1C, 0x1C, 0x1C, 0x00, // 3. 3/5 full progress block
  0x1E, 0x1E, 0x1E, 0x1F, 0x1E, 0x1E, 0x1E, 0x00, // 4. 4/5 full progress block
  0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, // 5. 5/5 full progress block
  0x0E, 0x15, 0x15, 0x17, 0x11, 0x11, 0x0E, 0x00, // 6. clock
  0x03, 0x05, 0x19, 0x11, 0x19, 0x05, 0x03, 0x00  // 5. dinamik fill
};
//=============================================================================
const uint8_t LcdBigNum[] PROGMEM = //define 8 custom LCD chars
{
  0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F,
  0x1F, 0x1F, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F,
  0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
  0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x00,
  0x1F, 0x1B, 0x13, 0x1B, 0x1B, 0x1B, 0x1F, 0x00,
  0x1F, 0x11, 0x1D, 0x11, 0x17, 0x11, 0x1F, 0x00,
  0x1F, 0x11, 0x1D, 0x11, 0x1D, 0x11, 0x1F, 0x00
};
//=============================================================================
void lcd(unsigned char p)
{ 
  LCD_PORTU |= (1 << LCD_EN);
  if (p & 0x80) { LCD_PORT |= (1 << LCD_D7); } else { LCD_PORT &= (~(1 << LCD_D7)); }
  if (p & 0x40) { LCD_PORT |= (1 << LCD_D6); } else { LCD_PORT &= (~(1 << LCD_D6)); }
  if (p & 0x20) { LCD_PORT |= (1 << LCD_D5); } else { LCD_PORT &= (~(1 << LCD_D5)); }
  if (p & 0x10) { LCD_PORT |= (1 << LCD_D4); } else { LCD_PORT &= (~(1 << LCD_D4)); }
  LCD_PORTU &= ~_BV(LCD_EN);
  _delay_ms(1);
  LCD_PORTU |= (1 << LCD_EN);
  if (p & 0x08) { LCD_PORT |= (1 << LCD_D7); } else { LCD_PORT &= (~(1 << LCD_D7)); }
  if (p & 0x04) { LCD_PORT |= (1 << LCD_D6); } else { LCD_PORT &= (~(1 << LCD_D6)); }
  if (p & 0x02) { LCD_PORT |= (1 << LCD_D5); } else { LCD_PORT &= (~(1 << LCD_D5)); }
  if (p & 0x01) { LCD_PORT |= (1 << LCD_D4); } else { LCD_PORT &= (~(1 << LCD_D4)); }
  LCD_PORTU &= (~(1 << LCD_EN));
  _delay_ms(1);
  LCD_PORTU |= (1 << LCD_EN);
}                
//=============================================================================
void LCD_com(unsigned char p)
{
  LCD_PORTU &= ~_BV(LCD_RS);
  LCD_PORTU |= _BV(LCD_EN);
  lcd(p);
}
//=============================================================================
void LCD_dat(unsigned char p)
{
  LCD_PORTU |= (1 << LCD_RS) | (1 << LCD_EN);
  lcd(p);
}
//=============================================================================
void LCD_init(void)
{
unsigned char ch = 0, chn = 0;
// настройка портов
LCD_DDR_LIGHT |= (1 << LCD_LIGHT);
LCD_PORT_LIGHT |= (1 << LCD_LIGHT);
LCD_DDR |= ((1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7));
LCD_PORT &= (~(1 << LCD_D4) | (1 << LCD_D5) | (1 << LCD_D6) | (1 << LCD_D7));
LCD_DDRU |= ((1 << LCD_RS) | (1 << LCD_EN) | (1 << LCD_RW)); 
LCD_PORTU &= (~(1 << LCD_RS) | (1 << LCD_EN) | (1 << LCD_RW)); 
_delay_ms(200);
LCD_com(0x33);
_delay_ms(20);
LCD_com(0x32);
_delay_ms(1);
LCD_com(0x28);
_delay_ms(1);
LCD_com(0x08);
_delay_ms(1);
LCD_com(0x01);
_delay_ms(50);
LCD_com(0x06);
LCD_com(0x0C);
while(ch < 64)
{
  LCD_definechar((LcdCustomChar + ch),chn++);
  ch = ch + 8;
}
}
//=============================================================================
void LCD_puts(char *data)
{
  while(*data) LCD_dat(*(data++));
}
//=============================================================================
void LCD_goto(unsigned char x, unsigned char y)
{
  if (y == 0) {
    LCD_com(0x80 + x);
  }
  if (y == 1) {
    LCD_com(0xC0 + x);
  }
}
//=============================================================================
void LCD_clear(void)
{
  LCD_com(1 << LCD_CLR);
  _delay_ms(200);
}
//=============================================================================
void LCD_definechar(const unsigned char *pc,uint8_t char_code)
{
  uint8_t a, pcc;
  uint16_t i;
  a = (char_code << 3) | 0x40;
  for (i = 0; i < 8; i++) {
    pcc = pgm_read_byte(&pc[i]);
	LCD_com(a++); LCD_dat(pcc);
  }
}
//=============================================================================
void LCD_progress_bar(unsigned char progress, unsigned char maxprogress, unsigned char length)
{
uint8_t i;
uint16_t pixelprogress;
uint8_t c;
pixelprogress = ((progress * (length * PROGRESSPIXELS_PER_CHAR)) / maxprogress);
for(i=0; i<length; i++)
{
  if( ((i * (uint16_t)PROGRESSPIXELS_PER_CHAR) + 5) > pixelprogress )
  {
    if( ((i*(uint16_t)PROGRESSPIXELS_PER_CHAR)) > pixelprogress )
    {
      c = 0;
    } else {
      c = pixelprogress % PROGRESSPIXELS_PER_CHAR;
    }
  } else {
    c = 5;
  }
  LCD_dat(c);
}

}
//=============================================================================
void lcd_space(unsigned char n_sp)
{
unsigned char i;
for (i = 1; i <= n_sp; i++) {
  LCD_dat(' ');
}
}
//=============================================================================
void LCD_load_defchar(void)
{
unsigned char ch = 0, chn = 0;
while(ch < 64)
{
  LCD_definechar((LcdCustomChar + ch),chn++);
  _delay_ms(1);
  ch = ch + 8;
}
}
//=============================================================================
void LCD_load_bignum(void)
{
unsigned char ch = 0, chn = 0;
while(ch < 64)
{
  LCD_definechar((LcdBigNum + ch),chn++);
  _delay_ms(1);
  ch = ch + 8;
}
}
//=============================================================================
