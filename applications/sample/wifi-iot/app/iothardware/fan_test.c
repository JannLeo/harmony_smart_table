/*
 * Copyright (c) 2020 Nanjing Xiaoxiongpai Intelligent Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
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
// #include "table_app.h"
#include "wifiiot_watchdog.h"
#define FAN_INTERVAL_TIME_US 300000
enum FanState
{
    Fan_ON = 0,
    Fan_OFF,
};
enum FanState state = Fan_OFF;
enum FanState nextstate = Fan_ON;
/***************************************************************
* 函数名称: Motor_StatusSet
* 说    明: 电机状态设置
* 参    数: status,ENUM枚举的数据
*									OFF,关
*									ON,开
* 返 回 值: 无
***************************************************************/
void Motor_StatusSet(char *arg)
{
    (void)arg;
    while(1){
        if (state == Fan_ON){
            // state = nextstate;
            // nextstate = Fan_OFF;
            
            printf("--------------------------FAN start!---------------------\r\n");
            //设置GPIO_8输出高电平打开电机
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_8, 1);
            usleep(FAN_INTERVAL_TIME_US);
        }
        else if (state == Fan_OFF){
            // state = nextstate;
            // nextstate = Fan_ON;
            printf("--------------------------FAN STOP!---------------------\r\n");
            //设置GPIO_8输出低电平关闭电机
            GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_8, 0);
            usleep(FAN_INTERVAL_TIME_US);
        }
    }
    
}
static void OnButtonPressed(char *arg)
{
    (void) arg;
    printf("--------------------------FAN PRESSED!---------------------\r\n");

     if (state == Fan_OFF){
        state = nextstate;
        nextstate = Fan_OFF;
    printf("--------------------------STATE ON!---------------------\r\n");
    }
    else if (state == Fan_ON){
        state = nextstate;
        nextstate = Fan_ON;
        printf("--------------------------STATE OFF!---------------------\r\n");
        
    }
}

    
static void StartfanTask(void)
{
    osThreadAttr_t attr;
    
    GpioInit();

    

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO); //设置引脚功能
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_PULL_UP);   //设置输出方向
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW,
                        OnButtonPressed, NULL);
    
    //设置GPIO_2的复用功能为普通GPIO
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);

    //设置GPIO_2为输出模式
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_GPIO_DIR_OUT);
    // WatchDogDisable();

    attr.name = "Motor_StatusSet";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)Motor_StatusSet, NULL, &attr) == NULL) {
        printf("[LedExample] Falied to create Motor_StatusSet!\n");
    }
}

APP_FEATURE_INIT(StartfanTask);