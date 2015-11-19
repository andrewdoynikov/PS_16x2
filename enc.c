#include "enc.h"
#include "rtos.h"
#include "events.h"
//=============================================================================
#define SetBit(port, bit) port |= (1 << bit)
#define ClearBit(port, bit) port &= ~(1 << bit)
//=============================================================================
#define b00000011 	3
#define b11010010 	210
#define b11100001 	225
//=============================================================================
void ENC_init(void)
{
  ClearBit(ENC_DDR, ENC_PIN1); // вход
  ClearBit(ENC_DDR, ENC_PIN2);
  SetBit(ENC_PORT, ENC_PIN1);  // вкл подтягивающий резистор
  SetBit(ENC_PORT, ENC_PIN2);
}
//=============================================================================
void ENC_poll(void)
{
  static unsigned char stateEnc; // хранит последовательность состояний энкодера
  unsigned char tmp;  
  unsigned char currentState = 0;

  // проверяем состояние выводов микроконтроллера
  if ((ENC_PIN & (1 << ENC_PIN1)) != 0) { SetBit(currentState, 0); }
  if ((ENC_PIN & (1 << ENC_PIN2)) != 0) { SetBit(currentState, 1); }
  // если равно предыдущему, то выходим
  tmp = stateEnc;
  if (currentState == (tmp & b00000011)) return;
  // если не равно, то сдвигаем и сохраняем в озу
  tmp = (tmp << 2) | currentState;
  stateEnc = tmp;
  // сравниваем получившуюся последовательность
  if (tmp == b11100001) {  RTOS_setTask(EVENT_KEY_LEFT, 0, 0); }
  if (tmp == b11010010) {  RTOS_setTask(EVENT_KEY_RIGHT, 0, 0); }
  return;
}

//=============================================================================

