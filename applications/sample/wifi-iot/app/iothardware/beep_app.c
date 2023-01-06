/*
这个代码api是对于按键蜂鸣   beep_on()
 */

#include <stdio.h>

#include <unistd.h>
#include "table_app.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_watchdog.h"

#define LED_INTERVAL_TIME_US 300000
#define LED_TASK_STACK_SIZE 1024
#define Beep_Task_PRIO 24

int g_sound_state = 0;
// enum LedState g_ledState = LED_SPARK;

void Button_demo1(char *arg)
{
    printf("g_sound_state=%d\r\n", g_sound_state);
    (void)arg;
    g_sound_state = !g_sound_state;
}

void *BeepTask(const char *arg)
{
    (void)arg;
    printf("beep start!\r\n");
    while (1)
    {

        if (g_sound_state)
        {

            PwmStart(WIFI_IOT_PWM_PORT_PWM0, 20 * 1000, 40 * 1000);
            usleep(LED_INTERVAL_TIME_US);
            PwmStop(WIFI_IOT_PWM_PORT_PWM0);
            g_sound_state = !g_sound_state;
            printf("beep!!!!!!\r\n");
        }
        else
        {
            printf("stop!!!!!!\r\n");

            PwmStart(WIFI_IOT_PWM_PORT_PWM0, 0, 40 * 1000);

            osDelay(100);
            // PwmStop(WIFI_IOT_PWM_PORT_PWM0);
        }
    }
    return NULL;
}

void beep_on(void)
{
    osThreadAttr_t attr;
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_PULL_UP);
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
                        Button_demo1, NULL);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM0);

    WatchDogDisable();
    printf("beep!!\r\n");
    attr.name = "BeepTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = LED_TASK_STACK_SIZE;
    attr.priority = Beep_Task_PRIO;

    if (osThreadNew((osThreadFunc_t)BeepTask, NULL, &attr) == NULL)
    {
        printf("[BeepTask] Falied to create LedTask!\n");
    }
}
