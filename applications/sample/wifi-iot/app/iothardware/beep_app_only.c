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

// enum LedState g_ledState = LED_SPARK;

void *Beep(const char *arg)
{
    (void)arg;
    PwmStart(WIFI_IOT_PWM_PORT_PWM0, 20 * 1000, 40 * 1000);
    usleep(1000000);
    PwmStop(WIFI_IOT_PWM_PORT_PWM0);

    return NULL;
}

void beep_only(void)
{
    // osThreadAttr_t attr;
    // // WIFI-IOT version pwm=1
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_PWM0_OUT);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);
    PwmInit(WIFI_IOT_PWM_PORT_PWM1);

    WatchDogDisable();
    PwmStart(WIFI_IOT_PWM_PORT_PWM1, 20 * 1000, 40 * 1000);
    usleep(1000000);
    PwmStop(WIFI_IOT_PWM_PORT_PWM1);


    printf("beep!!\n");
   
}

// APP_FEATURE_INIT(beep_only);