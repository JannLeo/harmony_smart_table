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
#include "wifiiot_pwm.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_watchdog.h"

#define PWM_TASK_STACK_SIZE 512
#define PWM_TASK_PRIO 25

static void PWMTask(void)
{
    unsigned int i;

    //初始化GPIO
    GpioInit();

    //设置GPIO_2引脚复用功能为PWM
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_IO_FUNC_GPIO_2_PWM2_OUT);

    //设置GPIO_2引脚为输出模式
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_2, WIFI_IOT_GPIO_DIR_OUT);

    //初始化PWM2端口
    PwmInit(WIFI_IOT_PWM_PORT_PWM2);

    while (1)
    {
        for (i = 0; i < 40000; i += 100)
        {
            //输出不同占空比的PWM波
            PwmStart(WIFI_IOT_PWM_PORT_PWM2, i, 40000);

            usleep(10);
        }
        i = 0;
    }
}

static void PWMExampleEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "PWMTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 512;
    attr.priority = 25;
    WatchDogDisable();
    printf("ready to beep!\n");
    if (osThreadNew((osThreadFunc_t)PWMTask, NULL, &attr) == NULL)
    {
        printf("Falied to create PWMTask!\n");
    }
}

APP_FEATURE_INIT(PWMExampleEntry);