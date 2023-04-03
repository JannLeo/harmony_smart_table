/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>

#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_pwm.h"
#include "wifiiot_watchdog.h"

#define LED_INTERVAL_TIME_US 300000
#define LED_TASK_STACK_SIZE 1024
#define LED_TASK_PRIO 25
uint32_t exec1;
enum LedState {
    LED_ON = 0,
    LED_OFF,
    LED_SPARK,
};
static int times = 0;
enum LedState nextstate=LED_ON;
//enum LedState g_ledState = LED_SPARK;
enum LedState g_ledState = LED_OFF;
osTimerId_t id1;
static void *ChildLock(const char *arg)
{
    (void)arg;
    while (1) {
        
        switch (g_ledState) {
            case LED_ON:
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 1);
                usleep(LED_INTERVAL_TIME_US);
                printf("0\r\n");
                break;
            case LED_OFF:
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 0);
                usleep(LED_INTERVAL_TIME_US);
                printf("1\r\n");
                break;
            case LED_SPARK:
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 0);
                usleep(LED_INTERVAL_TIME_US);
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 1);
                usleep(LED_INTERVAL_TIME_US);
                printf("2\r\n");
                break;
            default:
                usleep(LED_INTERVAL_TIME_US);
                printf("else\r\n");
                break;
        }
        printf("running!\r\n");
    }

    return NULL;
}
/***** 定时器1 回调函数 *****/
void Timer1_Callback(void *arg)
{
    (void)arg;
    printf("回调时钟函数！\r\n");
    
    times++;
    
}
static void Button_demo1(char *arg)
{
    (void)arg;
    
    uint32_t timerDelay;
    osStatus_t status;
    
    exec1 = 1U;
    id1 = osTimerNew(Timer1_Callback, osTimerPeriodic, &arg, NULL);
    printf("中断,id1= %d \r\n",id1);
    if (id1 == NULL)
    {
        printf("[Timer Test] osTimerNew(periodic timer) failed.\r\n");
        return;
    } else{
        printf("创建时钟成功!\r\n");
    }
    // Hi3861 1U=10ms,100U=1S
    timerDelay = 300U;
    
    status = osTimerStart(id1, timerDelay);
    if (status != osOK)
    {
        printf("时钟不合规矩！\r\n");
        return;
        // Timer could not be started
    }else {
        printf("时钟合规！\r\n");
    }
    osDelay(100);

    while(times<=300){
        int signal_press=GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_5,0);
        printf("signal_press=%d\r\n",signal_press);
        if(signal_press==0){
            g_ledState=nextstate;
        printf("变灯！!\r\n");
        }
        osDelay(100);
    }
    status = osTimerStop(id1);
    printf("[Timer Test] stop periodic timer, status :%d.\r\n", status);
    status = osTimerDelete(id1);
    printf("[Timer Test] kill periodic timer, status :%d.\r\n", status);

}

static void Child_lock_ex1(void)
{
    osThreadAttr_t attr;
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_IO_FUNC_GPIO_2_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_GPIO_DIR_OUT);
    
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11,WIFI_IOT_IO_FUNC_GPIO_11_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11,WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_11,WIFI_IOT_IO_PULL_UP);
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_11,WIFI_IOT_INT_TYPE_EDGE,WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
        Button_demo1,NULL);
    
    WatchDogDisable();

    attr.name = "ChildLock";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = LED_TASK_STACK_SIZE;
    attr.priority = LED_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)ChildLock, NULL, &attr) == NULL) {
        printf("[ChildLock] Falied to create LedTask!\n");
    }
}

APP_FEATURE_INIT(Child_lock_ex1);