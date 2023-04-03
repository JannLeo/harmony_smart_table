#ifndef __APP_DEMO_SYSINFO_H__
#define __APP_DEMO_SYSINFO_H__

/**
* @brief  monitor system information and task infomation 
* @param None
* @return None
*/
void smarttable_heap_task(void);
int get_Memory_Usage(void);
int get_Max_Memory_Usage(void);
int get_Timer_Usage(void);
int get_Task_Usage(void);
int get_Sem_Usage(void);
int get_Queue_Usage(void);
int get_Mux_Usage(void);
int get_Event_Usage(void);
#endif