#include <hi_watchdog.h>
#include <hi_types_base.h>
#include "hi_os_stat.h"
#include "hi_mem.h"
#include "los_task_pri.h"
#include "hi_stdlib.h"
#include "hi_task.h"
#include <hi_time.h>
#include "hi_at.h"
#include "stdio.h"
#include "los_task.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "SmartTable_Sysinfo.h"

#define HEAP_TASK_STAK_SIZE     4096
#define HEAP_TASK_PRIORITY      25

void smarttable_heap_task(void)
{
    hi_os_resource_use_stat os_resource_stat = {0};
    hi_mdm_mem_info mem_inf = {0};
    
    TSK_INFO_S* ptask_info = HI_NULL;
    char task_info[256];
    hi_u32 len = 0;
    hi_u32 i = 0;
    memset_s(task_info, sizeof(task_info), 0, sizeof(task_info));
    len = 0;
    (hi_void)hi_os_get_resource_status(&os_resource_stat); //get system resource info
    (hi_void)hi_mem_get_sys_info(&mem_inf); //get total mem info
    if(mem_inf.total + mem_inf.total_lmp-mem_inf.peek_size <= 100000){
        printf("内存存储空间不足，请清理！\r\n");
    }
    len = sprintf_s(task_info + len, 256-len, "+profiling\r\n");  //start flag
    len += sprintf_s(task_info + len, 256-len, "rtc=%d\r\n", hi_get_milli_seconds());  //insert time info  
    len += sprintf_s(task_info + len, 256-len, "mem=%d,", (mem_inf.total + mem_inf.total_lmp)); //总内存   
    len += sprintf_s(task_info + len, 256-len, "%d,", (mem_inf.used + mem_inf.used_lmp)); //已使用内存
    len += sprintf_s(task_info + len, 256-len, "%d\r\n", mem_inf.peek_size); //峰值占用内存
    len += sprintf_s(task_info + len, 256-len, "os_resource=");
    len += sprintf_s(task_info + len, 256-len, "%d,", os_resource_stat.timer_usage); //定时器个数
    len += sprintf_s(task_info + len, 256-len, "%d,", os_resource_stat.task_usage); //任务个数
    len += sprintf_s(task_info + len, 256-len, "%d,", os_resource_stat.sem_usage); //信号量个数
    len += sprintf_s(task_info + len, 256-len, "%d,", os_resource_stat.queue_usage); //队列个数
    len += sprintf_s(task_info + len, 256-len, "%d,", os_resource_stat.mux_usage); // 互斥锁个数
    len += sprintf_s(task_info + len, 256-len, "%d\r\n", os_resource_stat.event_usage); //事件个数
    ptask_info = (TSK_INFO_S*)hi_malloc(HI_MOD_ID_SAL_DFX, sizeof(TSK_INFO_S));
    if (ptask_info != HI_NULL) {
        for (i = 0; i < g_taskMaxNum; i++) {
            memset_s(ptask_info, sizeof(TSK_INFO_S), 0, sizeof(TSK_INFO_S));
            hi_u32 ret = LOS_TaskInfoGet(i, ptask_info); //check every task info one by one
            if (ret == HI_ERR_SUCCESS) {

                if (len >= 200) {
                    printf("%s", task_info); //printf while length is in 200-256
                    len = 0;
                }
                len += sprintf_s(task_info + len, 256-len, "task=%s,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\r\n",
                            ptask_info->acName, ptask_info->uwTaskID, ptask_info->usTaskStatus, ptask_info->usTaskPrio,
                            ptask_info->uwStackSize, ptask_info->uwCurrUsed, ptask_info->uwPeakUsed);
            }
        }
        hi_free(HI_MOD_ID_SAL_DFX, ptask_info);
    }
    else
    {
        printf("malloc ptask_info failed\r\n");
    }
    len = sprintf_s(task_info + len, 256-len, "-profiling\r\n");  //end flag
    printf("%s\r\n", task_info);
    hi_watchdog_feed();
}
int get_Memory_Usage(void){
    hi_mdm_mem_info mem_inf = {0};
    (hi_void)hi_mem_get_sys_info(&mem_inf); //get total mem info
    if(mem_inf.total + mem_inf.total_lmp-mem_inf.peek_size <= 100000){
        printf("内存峰值存储空间不足，请清理！\r\n");
    }
    hi_watchdog_feed();
    return (mem_inf.total + mem_inf.total_lmp-mem_inf.peek_size)/1000;
    
    
}
int get_Max_Memory_Usage(void){
    hi_mdm_mem_info mem_inf = {0};
    (hi_void)hi_mem_get_sys_info(&mem_inf); //get total mem info
    if(mem_inf.total + mem_inf.total_lmp-mem_inf.used - mem_inf.used_lmp <= 100000){
        printf("内存存储空间不足，请清理！\r\n");
    }
    hi_watchdog_feed();
    return (mem_inf.total + mem_inf.total_lmp-mem_inf.used - mem_inf.used_lmp)/1000;
    
    
}
int get_Timer_Usage(void){
    hi_os_resource_use_stat os_resource_stat = {0};
    (hi_void)hi_os_get_resource_status(&os_resource_stat); //get system resource info
    return os_resource_stat.timer_usage;
    
    hi_watchdog_feed();
}
int get_Task_Usage(void){
    hi_os_resource_use_stat os_resource_stat = {0};
    (hi_void)hi_os_get_resource_status(&os_resource_stat);
    hi_watchdog_feed();
    return os_resource_stat.task_usage;
    
    
}
int get_Sem_Usage(void){
    hi_os_resource_use_stat os_resource_stat = {0};
    (hi_void)hi_os_get_resource_status(&os_resource_stat);
    hi_watchdog_feed();
    return os_resource_stat.sem_usage;
    
    
}
int get_Queue_Usage(void){
    hi_os_resource_use_stat os_resource_stat = {0};
    (hi_void)hi_os_get_resource_status(&os_resource_stat); 
    hi_watchdog_feed();
    return os_resource_stat.queue_usage;
    
    
}
int get_Mux_Usage(void){
    hi_os_resource_use_stat os_resource_stat = {0};
    (hi_void)hi_os_get_resource_status(&os_resource_stat);
    hi_watchdog_feed();
    return os_resource_stat.mux_usage;
    
    
}
int get_Event_Usage(void){
    hi_os_resource_use_stat os_resource_stat = {0};
    (hi_void)hi_os_get_resource_status(&os_resource_stat);
    hi_watchdog_feed();
    return os_resource_stat.event_usage;
    
    
}