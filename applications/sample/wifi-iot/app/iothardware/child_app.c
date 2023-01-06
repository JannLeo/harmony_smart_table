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
#define LED_TASK_STACK_SIZE 1024*4
#define LED_TASK_PRIO 25
uint32_t exec1;
enum LedState {
    LED_ON = 0,
    LED_OFF,
    LED_SPARK,
};
enum LedState nextstate=LED_ON;
//enum LedState g_ledState = LED_SPARK;
enum LedState g_ledState = LED_OFF;
int times=0;
static void *ChildLock(const char *arg)
{
    (void)arg;
    while (1) {
        IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
        GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);
        switch (g_ledState) {
            case LED_ON:
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 1);
                usleep(LED_INTERVAL_TIME_US);
                printf("0\r\n");
                break;
            case LED_OFF:
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 0);
                usleep(LED_INTERVAL_TIME_US);
                printf("1\r\n");
                break;
            case LED_SPARK:
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 0);
                usleep(LED_INTERVAL_TIME_US);
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 1);
                usleep(LED_INTERVAL_TIME_US);
                printf("2\r\n");
                break;
            default:
                usleep(LED_INTERVAL_TIME_US);
                printf("else\r\n");
                break;
        }
        printf("times=%d\r\n",times);
        if(times>1){
            times=1;
        }
        if(times==1){
            printf("进入轮回！！\r\n");
            int t=0;
            while(t<3){
                t++;
                printf("t=%d,小于等于3！\r\n",t);
                osDelay(100);
                if(times==2){
                    printf("按键时长不够，失败！\r\n");
                    GpioSetIsrMode(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_INT_TYPE_EDGE,WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW);
                    times=0;
                }    
            }
            if(times==1){
                 printf("按键时长足够，成功！\r\n");
                 GpioSetIsrMode(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_INT_TYPE_EDGE,WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW);
                 times=0;
                 if(g_ledState!=nextstate)
                    g_ledState=nextstate;
                else
                    g_ledState=LED_OFF;
                 
            }
            

        }
        // unsigned int signal_press=GpioGetDir(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_GPIO_DIR_IN);
        // printf("signal_press=%d\r\n",signal_press);
        // if(signal_press==0){
        //     printf("成功进入判断分支！\r\n");
        //     osDelay(300);
        //     printf("延时完成！\r\n");
        //     printf("signal_press=%d\r\n",signal_press);
        //     if(signal_press==0){
        //         printf("成功进入亮灯分支！\r\n");
        //         if(g_ledState!=nextstate)
        //             g_ledState=nextstate;
        //         else
        //             g_ledState=LED_OFF;
        // }
        // }
        printf("running!\r\n");
    }

    return NULL;
}
static void Button_demo1(char *arg)
{
    (void)arg;
    times++;
    GpioSetIsrMode(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_INT_TYPE_EDGE,WIFI_IOT_GPIO_EDGE_RISE_LEVEL_HIGH);
    
}

static void Child_lock_ex1(void)
{
    osThreadAttr_t attr;
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_IO_PULL_UP);
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_INT_TYPE_EDGE,WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
        Button_demo1,NULL);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);
    
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

SYS_RUN(Child_lock_ex1);