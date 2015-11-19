#include <avr/io.h>
#include <avr/interrupt.h>
#include "rtos.h"
#include "events.h"
#include "interface.h"
//=============================================================================
void (*pState)(unsigned char event) = INITIAL_STATE;

unsigned char timerInterrupt = 0; 			// ���� ���������� �������
volatile static task TaskArray[MAX_TASKS]; 	// ������� �����
volatile static unsigned char arrayTail;   	// "�����" �������

static volatile unsigned char saveRegister;

#define ENABLE_INTERRUPT do { sei(); } while(0)
#define DISABLE_INTERRUPT do { saveRegister = SREG; cli(); } while(0)
#define RESTORE_INTERRUPT do { SREG = saveRegister; } while(0) // ������������ RESTORE ������ ����� DISABLE
//=============================================================================
void RTOS_init()
{
  TCCR2B |= (1 << CS22);        // ��������� - 64
  TIFR2 = (1 << TOV2);          // ������� ���� ���������� ������� �2
  TIMSK2 |= (1 << TOIE2);       // ��������� ���������� �� ������������
  TIMER_COUNTER = TIMER_START;  // ��������� ��������� ��. � ������� �������
  arrayTail = 0;                // "�����" � 0
}
//=============================================================================
// ��������� ������
void RTOS_setTask (unsigned char event, unsigned int taskDelay, unsigned int taskPeriod)
{
  unsigned char i;
  if (!event) return;
  for(i = 0; i < arrayTail; i++)              // ����� ������ � ������� ������
  {
    if (TaskArray[i].event == event)          // ���� �����, �� ��������� ����������
    {
      TaskArray[i].pFunc  = 0;
      TaskArray[i].delay  = taskDelay;
      TaskArray[i].period = taskPeriod;
      if (!taskDelay) {                       // ���� �������� ������� == 0
        TaskArray[i].run = 1;                 // �� ����� ������������� ���� �������
      } else TaskArray[i].run = 0;   
      return;                                 // �������, �������
    }
  }
  if (arrayTail < MAX_TASKS)                  // ���� ����� ������ � ������ ��� 
  {                                           // � ���� �����,�� ���������
    TaskArray[i].pFunc  = 0;
    TaskArray[arrayTail].event  = event;
    TaskArray[arrayTail].delay  = taskDelay;
    TaskArray[arrayTail].period = taskPeriod;
    if (!taskDelay) {                         // ���� �������� ������� == 0
      TaskArray[i].run = 1;                   // �� ����� ������������� ���� �������
    } else TaskArray[i].run = 0;   
    arrayTail++;                              // ����������� "�����"
  } 
}
//=============================================================================
 // ��������� ������
void RTOS_setTaskFunc (void (*taskFunc)(void), unsigned int taskDelay, unsigned int taskPeriod)
{
  unsigned char i;
  if(!taskFunc) return;
  for(i = 0; i < arrayTail; i++)              // ����� ������ � ������� ������
  {
    if(TaskArray[i].pFunc == taskFunc)        // ���� �����, �� ��������� ����������
    {
      DISABLE_INTERRUPT;
      TaskArray[i].event  = EVENT_NULL;
      TaskArray[i].delay  = taskDelay;
      TaskArray[i].period = taskPeriod;
      TaskArray[i].run    = 0;   
      RESTORE_INTERRUPT;
      return;                                 // �������, �������
    }
  }
  if (arrayTail < MAX_TASKS)                  // ���� ����� ������ � ������ ��� 
  {                                           // � ���� �����,�� ���������
    DISABLE_INTERRUPT;
    TaskArray[arrayTail].pFunc  = taskFunc;
    TaskArray[arrayTail].event  = EVENT_NULL;
    TaskArray[arrayTail].delay  = taskDelay;
    TaskArray[arrayTail].period = taskPeriod;
    TaskArray[arrayTail].run    = 0;   
    arrayTail++;                              // ����������� "�����"
    RESTORE_INTERRUPT;
  }
}
//=============================================================================
// �������� ������ �� �������
void RTOS_deleteTask (unsigned char event)
{
  unsigned char i;
  for (i = 0; i < arrayTail; i++)             // �������� �� ������ �����
  {
    if (TaskArray[i].event == event)          // ���� ������ � ������ �������
    {
      if (i != (arrayTail - 1))               // ��������� ��������� ������
      {                                       // �� ����� ���������
        TaskArray[i] = TaskArray[arrayTail - 1];
      }
      arrayTail--;                            // ��������� ��������� "������"
      return;
    }
  }
}
//=============================================================================
// �������� ������ �� �������
void RTOS_deleteTaskFunc (void (*taskFunc)(void)) 
{
  unsigned char i;
  for (i=0; i<arrayTail; i++)                      // �������� �� ������ �����
  {
    if(TaskArray[i].pFunc == taskFunc)             // ���� ������ � ������ �������
    {
      DISABLE_INTERRUPT;
      if(i != (arrayTail - 1))                     // ��������� ��������� ������
      {                                            // �� ����� ���������
        TaskArray[i] = TaskArray[arrayTail - 1];
      }
      arrayTail--;                                 // ��������� ��������� "������"
      RESTORE_INTERRUPT;
      return;
    }
  }
}
//=============================================================================
void RTOS_dispatchTask()
{
  unsigned char i, temp;
  void (*function) (void);
  if (timerInterrupt)
  {  
    timerInterrupt = 0;                      // ���������� ���� ����������
    for (i = 0; i < arrayTail; i++)          // �������� �� ������ �����
    { 
	  if (TaskArray[i].delay == 0) {         // ���� ����� �� ���������� �������
        TaskArray[i].run = 1;                // ������� ���� �������,
      } else TaskArray[i].delay--;           // ����� ��������� �����
    }
  }
  for (i = 0; i < arrayTail; i++)            // �������� �� ������ �����
  {
    if (TaskArray[i].run == 1)               // ���� ���� �� ���������� �������,
    {                                        // ���������� ������, �.�. ��
      temp = TaskArray[i].event;             // ����� ���������� ����� 
      function = TaskArray[i].pFunc;   
      if(TaskArray[i].period == 0)                 
      {  
        cli();
	    if (temp == EVENT_NULL) {
          RTOS_deleteTaskFunc(TaskArray[i].pFunc); // ������� ������ �� ������,
	    } else {
          RTOS_deleteTask(TaskArray[i].event); // ������� ������ �� ������,
		}
        sei();
      } else {
        TaskArray[i].run = 0;                  // ����� ������� ���� �������
        if (!TaskArray[i].delay)               // ���� ������ �� �������� ��������
        {                                      // ������ ��
          TaskArray[i].delay = TaskArray[i].period - 1; 
        }                                      // ������ ��� ���� ����� ������� �����  
      }
	  if (temp == EVENT_NULL) {
        if(!function) return;
        (*function)();                         // ��������� ������
	  } else {
        pState(temp);                          // ��������� ������
	  }
    }
  }
}
//=============================================================================
ISR(TIMER_ISR)
{
  TIMER_COUNTER = TIMER_START;               // ������ ��������� �������� �������
  timerInterrupt = 1;                        // ������������� ���� ���������� 
}
//=============================================================================
//=============================================================================
