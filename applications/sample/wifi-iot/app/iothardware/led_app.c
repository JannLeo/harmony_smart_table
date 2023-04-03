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
#include "table_app.h"
#include "wifiiot_watchdog.h"

#include "beep_only.h"
#include "wifiiot_errno.h"

// #include "beep_only.h"
#define LED_INTERVAL_TIME_US 300000
// #define LED_INTERVAL_TIME_US 1000000
#define LED_TASK_STACK_SIZE 1024
#define LED_TASK_PRIO 24

enum LedState
{
    LED_ON = 0,
    LED_OFF,
    LED_SPARK,
};
int LED_states_leo = 0; 
// enum LedState g_ledState = LED_SPARK;
enum LedState g_ledState = LED_OFF;
enum LedState nextstate = LED_ON;
//通过arg来传递参数，指向哪个GPIO口，然后对应的转变状态，状态由结构体LED_states指出
void Button_demo2(char *arg)
{
    (void)arg;
    
    // enum LedState nextstate = LED_SPARK;
    // IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    // GpioSetDir(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_GPIO_DIR_OUT);
    switch (g_ledState)
    {
    case LED_ON:
        nextstate = LED_OFF;
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 0);//输出低电平
        usleep(LED_INTERVAL_TIME_US);
        printf("led on\r\n");
        break;
    case LED_OFF:
        nextstate = LED_ON;
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 1);//输出高电平
        usleep(LED_INTERVAL_TIME_US);
        printf("led off\r\n");
        break;
    default:
        usleep(LED_INTERVAL_TIME_US);
        printf("---------------------LED error--------------------- \r\n");
        break;
    }
    
    LED_states_leo++;
    g_ledState = nextstate;
}

void led_app(void)
{
    // osThreadAttr_t attr;
    // //WIFI-IOT version
    // GpioInit();
    // IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    // IoSetPull(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_IO_PULL_UP);
    // GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_5, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
    //                     Button_demo2, NULL);
    // IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    // GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);

    // GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 0);
    // usleep(LED_INTERVAL_TIME_US);

    //BearPiNano version
    // LED_state = 0;
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO); //设置引脚功能
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_PULL_UP);   //设置输出方向
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
                        Button_demo2, NULL);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_IO_FUNC_GPIO_2_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_GPIO_DIR_OUT);
    // if( LED_states_leo > 0 ){
    //     printf("--------------------------LED beep!---------------------\r\n");
    //     beep_only();
    //     LED_states_leo = 0;
    // }
    printf("--------------------------LED app!---state= %d ------------------\r\n",LED_states_leo);
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 0);
    usleep(LED_INTERVAL_TIME_US);
}
