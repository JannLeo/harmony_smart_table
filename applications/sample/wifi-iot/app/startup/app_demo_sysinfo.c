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
#include "app_demo_sysinfo.h"

#define HEAP_TASK_STAK_SIZE     4096
#define HEAP_TASK_PRIORITY      25

static void *app_heap_task(const char *arg)
{
    (void)arg;
    hi_os_resource_use_stat os_resource_stat = {0};
    hi_mdm_mem_info mem_inf = {0};
    
    TSK_INFO_S* ptask_info = HI_NULL;
    char task_info[256];
    hi_u32 len = 0;
    hi_u32 i = 0;
    while(1)
    {
        memset_s(task_info, sizeof(task_info), 0, sizeof(task_info));
        len = 0;

        (hi_void)hi_os_get_resource_status(&os_resource_stat); //get system resource info
        (hi_void)hi_mem_get_sys_info(&mem_inf); //get total mem info

        len = sprintf_s(task_info + len, 256-len, "+profiling\r\n");  //start flag
        
        len += sprintf_s(task_info + len, 256-len, "rtc=%d\r\n", hi_get_milli_seconds());  //insert time info
        
        len += sprintf_s(task_info + len, 256-len, "mem=%d,", (mem_inf.total + mem_inf.total_lmp)); //total   
        len += sprintf_s(task_info + len, 256-len, "%d,", (mem_inf.used + mem_inf.used_lmp)); //used
        len += sprintf_s(task_info + len, 256-len, "%d\r\n", mem_inf.peek_size); //peak size

        len += sprintf_s(task_info + len, 256-len, "os_resource=");
        len += sprintf_s(task_info + len, 256-len, "%d,", os_resource_stat.timer_usage); //timer usage
        len += sprintf_s(task_info + len, 256-len, "%d,", os_resource_stat.task_usage); //task usage
        len += sprintf_s(task_info + len, 256-len, "%d,", os_resource_stat.sem_usage); //sem usage
        len += sprintf_s(task_info + len, 256-len, "%d,", os_resource_stat.queue_usage); //queue usage
        len += sprintf_s(task_info + len, 256-len, "%d,", os_resource_stat.mux_usage); // mux usage
        len += sprintf_s(task_info + len, 256-len, "%d\r\n", os_resource_stat.event_usage); //event usage

        
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

        hi_sleep(100); //100ms monitor cycle
        hi_watchdog_feed();
    }
    return NULL;
}

void app_demo_heap_task(void)
{
    osThreadAttr_t attr;
    attr.name = "app_heap_task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = HEAP_TASK_STAK_SIZE;
    attr.priority = HEAP_TASK_PRIORITY;
    if (osThreadNew((osThreadFunc_t)app_heap_task, NULL, &attr) == NULL) { //creat task
        printf("Failed to creat heap task!\n");
    }
    app_demo_heap_task();
    return;
}
// APP_FEATURE_INIT(app_demo_heap_task);

