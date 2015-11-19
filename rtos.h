#ifndef RTOS_H
#define RTOS_H	          
//=============================================================================
#define MAX_TASKS       16              // ���������� �����
#define TIMER_COUNTER   TCNT2           // �������-�������
#define TIMER_START     130             // ��������� ��� ������� �2
#define TIMER_ISR       TIMER2_OVF_vect // ������ ���������� �� ������������ �2
// ��������� ��������� 
#define INITIAL_STATE   run_main
//=============================================================================
typedef struct task
{   
  void (*pFunc) (void); // ��������� �� �������
  unsigned char event;  // �������
  unsigned int delay;   // �������� ����� ������ �������� ������
  unsigned int period;  // ������ ������� ������
  unsigned char run;    // ���� ���������� ������ � �������
} task;
//=============================================================================
void RTOS_init (void);	// ������������� ����������
void RTOS_setTask (unsigned char event, unsigned int taskDelay, unsigned int taskPeriod); // ��������� ������
void RTOS_setTaskFunc (void (*taskFunc)(void), unsigned int taskDelay, unsigned int taskPeriod); // ��������� ������
void RTOS_deleteTask (unsigned char event); // �������� ������ �� �������
void RTOS_deleteTaskFunc (void (*taskFunc)(void)); // �������� ������ �� �������
void RTOS_dispatchTask (void); // �������� ������ ����������, ���������� � ������� ����� 
//=============================================================================
#endif
