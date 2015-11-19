#include <avr/io.h>
#include <avr/interrupt.h>
#include "rtos.h"
#include "events.h"
#include "interface.h"
//=============================================================================
void (*pState)(unsigned char event) = INITIAL_STATE;

unsigned char timerInterrupt = 0; 			// Флаг прерывания таймера
volatile static task TaskArray[MAX_TASKS]; 	// очередь задач
volatile static unsigned char arrayTail;   	// "хвост" очереди

static volatile unsigned char saveRegister;

#define ENABLE_INTERRUPT do { sei(); } while(0)
#define DISABLE_INTERRUPT do { saveRegister = SREG; cli(); } while(0)
#define RESTORE_INTERRUPT do { SREG = saveRegister; } while(0) // использовать RESTORE только после DISABLE
//=============================================================================
void RTOS_init()
{
  TCCR2B |= (1 << CS22);        // прескалер - 64
  TIFR2 = (1 << TOV2);          // очищаем флаг прерывания таймера Т2
  TIMSK2 |= (1 << TOIE2);       // разрешаем прерывание по переполнению
  TIMER_COUNTER = TIMER_START;  // загружаем начальное зн. в счетный регистр
  arrayTail = 0;                // "хвост" в 0
}
//=============================================================================
// Установка задачи
void RTOS_setTask (unsigned char event, unsigned int taskDelay, unsigned int taskPeriod)
{
  unsigned char i;
  if (!event) return;
  for(i = 0; i < arrayTail; i++)              // поиск задачи в текущем списке
  {
    if (TaskArray[i].event == event)          // если нашли, то обновляем переменные
    {
      TaskArray[i].pFunc  = 0;
      TaskArray[i].delay  = taskDelay;
      TaskArray[i].period = taskPeriod;
      if (!taskDelay) {                       // если задержка запуска == 0
        TaskArray[i].run = 1;                 // то сразу устанавливаем флаг запуска
      } else TaskArray[i].run = 0;   
      return;                                 // обновив, выходим
    }
  }
  if (arrayTail < MAX_TASKS)                  // если такой задачи в списке нет 
  {                                           // и есть место,то добавляем
    TaskArray[i].pFunc  = 0;
    TaskArray[arrayTail].event  = event;
    TaskArray[arrayTail].delay  = taskDelay;
    TaskArray[arrayTail].period = taskPeriod;
    if (!taskDelay) {                         // если задержка запуска == 0
      TaskArray[i].run = 1;                   // то сразу устанавливаем флаг запуска
    } else TaskArray[i].run = 0;   
    arrayTail++;                              // увеличиваем "хвост"
  } 
}
//=============================================================================
 // Установка задачи
void RTOS_setTaskFunc (void (*taskFunc)(void), unsigned int taskDelay, unsigned int taskPeriod)
{
  unsigned char i;
  if(!taskFunc) return;
  for(i = 0; i < arrayTail; i++)              // поиск задачи в текущем списке
  {
    if(TaskArray[i].pFunc == taskFunc)        // если нашли, то обновляем переменные
    {
      DISABLE_INTERRUPT;
      TaskArray[i].event  = EVENT_NULL;
      TaskArray[i].delay  = taskDelay;
      TaskArray[i].period = taskPeriod;
      TaskArray[i].run    = 0;   
      RESTORE_INTERRUPT;
      return;                                 // обновив, выходим
    }
  }
  if (arrayTail < MAX_TASKS)                  // если такой задачи в списке нет 
  {                                           // и есть место,то добавляем
    DISABLE_INTERRUPT;
    TaskArray[arrayTail].pFunc  = taskFunc;
    TaskArray[arrayTail].event  = EVENT_NULL;
    TaskArray[arrayTail].delay  = taskDelay;
    TaskArray[arrayTail].period = taskPeriod;
    TaskArray[arrayTail].run    = 0;   
    arrayTail++;                              // увеличиваем "хвост"
    RESTORE_INTERRUPT;
  }
}
//=============================================================================
// Удаление задачи из очереди
void RTOS_deleteTask (unsigned char event)
{
  unsigned char i;
  for (i = 0; i < arrayTail; i++)             // проходим по списку задач
  {
    if (TaskArray[i].event == event)          // если задача в списке найдена
    {
      if (i != (arrayTail - 1))               // переносим последнюю задачу
      {                                       // на место удаляемой
        TaskArray[i] = TaskArray[arrayTail - 1];
      }
      arrayTail--;                            // уменьшаем указатель "хвоста"
      return;
    }
  }
}
//=============================================================================
// Удаление задачи из очереди
void RTOS_deleteTaskFunc (void (*taskFunc)(void)) 
{
  unsigned char i;
  for (i=0; i<arrayTail; i++)                      // проходим по списку задач
  {
    if(TaskArray[i].pFunc == taskFunc)             // если задача в списке найдена
    {
      DISABLE_INTERRUPT;
      if(i != (arrayTail - 1))                     // переносим последнюю задачу
      {                                            // на место удаляемой
        TaskArray[i] = TaskArray[arrayTail - 1];
      }
      arrayTail--;                                 // уменьшаем указатель "хвоста"
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
    timerInterrupt = 0;                      // сбрасываем флаг прерывания
    for (i = 0; i < arrayTail; i++)          // проходим по списку задач
    { 
	  if (TaskArray[i].delay == 0) {         // если время до выполнения истекло
        TaskArray[i].run = 1;                // взводим флаг запуска,
      } else TaskArray[i].delay--;           // иначе уменьшаем время
    }
  }
  for (i = 0; i < arrayTail; i++)            // проходим по списку задач
  {
    if (TaskArray[i].run == 1)               // если флаг на выполнение взведен,
    {                                        // запоминаем задачу, т.к. во
      temp = TaskArray[i].event;             // время выполнения может 
      function = TaskArray[i].pFunc;   
      if(TaskArray[i].period == 0)                 
      {  
        cli();
	    if (temp == EVENT_NULL) {
          RTOS_deleteTaskFunc(TaskArray[i].pFunc); // удаляем задачу из списка,
	    } else {
          RTOS_deleteTask(TaskArray[i].event); // удаляем задачу из списка,
		}
        sei();
      } else {
        TaskArray[i].run = 0;                  // иначе снимаем флаг запуска
        if (!TaskArray[i].delay)               // если задача не изменила задержку
        {                                      // задаем ее
          TaskArray[i].delay = TaskArray[i].period - 1; 
        }                                      // задача для себя может сделать паузу  
      }
	  if (temp == EVENT_NULL) {
        if(!function) return;
        (*function)();                         // выполняем задачу
	  } else {
        pState(temp);                          // выполняем задачу
	  }
    }
  }
}
//=============================================================================
ISR(TIMER_ISR)
{
  TIMER_COUNTER = TIMER_START;               // задаем начальное значение таймера
  timerInterrupt = 1;                        // устанавливаем флаг прерывания 
}
//=============================================================================
//=============================================================================
