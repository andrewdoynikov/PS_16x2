#ifndef RTOS_H
#define RTOS_H	          
//=============================================================================
#define MAX_TASKS       16              // Количество задач
#define TIMER_COUNTER   TCNT2           // Регистр-счетчик
#define TIMER_START     130             // Константа для таймера Т2
#define TIMER_ISR       TIMER2_OVF_vect // Вектор прерывания по переполнению Т2
// Начальное состояние 
#define INITIAL_STATE   run_main
//=============================================================================
typedef struct task
{   
  void (*pFunc) (void); // указатель на функцию
  unsigned char event;  // событие
  unsigned int delay;   // задержка перед первым запуском задачи
  unsigned int period;  // период запуска задачи
  unsigned char run;    // флаг готовности задачи к запуску
} task;
//=============================================================================
void RTOS_init (void);	// Инициализация диспетчера
void RTOS_setTask (unsigned char event, unsigned int taskDelay, unsigned int taskPeriod); // Установка задачи
void RTOS_setTaskFunc (void (*taskFunc)(void), unsigned int taskDelay, unsigned int taskPeriod); // Установка задачи
void RTOS_deleteTask (unsigned char event); // Удаление задачи из очереди
void RTOS_deleteTaskFunc (void (*taskFunc)(void)); // Удаление задачи из очереди
void RTOS_dispatchTask (void); // Основная работа диспетчера, вызывается в главном цикле 
//=============================================================================
#endif
