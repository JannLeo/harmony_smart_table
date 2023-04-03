#include "table_app.h"
#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"

static void beep_oo(void)
{
    osDelay(300);
    printf("beep press start!\r\n");
    beep_on(); //按键蜂鸣一声
    // printf("beep one start!!\r\n");
    // beep_only(); //蜂鸣一声
    // printf("led press start!!\r\n");
    // led_app(); //按键亮灯
    printf("5\r\n");
}
uint32_t exec1, exec2;

/***** 定时器1 回调函数 *****/
void Timer1_Callback(void *arg)
{
    (void)arg;
    printf("This is BearPi Harmony Timer1_Callback!\r\n");
}

/***** 定时器2 回调函数 *****/
void Timer2_Callback(void *arg)
{
    (void)arg;
    printf("This is BearPi Harmony Timer2_Callback!\r\n");
}

/***** 定时器创建 *****/
static void Timer_example(void)
{
    osTimerId_t id1, id2;
    uint32_t timerDelay;
    osStatus_t status;

    exec1 = 1U;
    id1 = osTimerNew(Timer1_Callback, osTimerPeriodic, &exec1, NULL);
    if (id1 != NULL)
    {

        // Hi3861 1U=10ms,100U=1S
        timerDelay = 100U;

        status = osTimerStart(id1, timerDelay);
        if (status != osOK)
        {
            // Timer could not be started
        }
    }

    exec2 = 1U;
    id2 = osTimerNew(Timer2_Callback, osTimerPeriodic, &exec2, NULL);
    if (id2 != NULL)
    {

        // Hi3861 1U=10ms,300U=3S
        timerDelay = 300U;

        status = osTimerStart(id2, timerDelay);
        if (status != osOK)
        {
            // Timer could not be started
        }
    }
}

APP_FEATURE_INIT(Timer_example);

// APP_FEATURE_INIT(beep_oo);