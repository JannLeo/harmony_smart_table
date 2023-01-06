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
static void *ChildLock(const char *arg)
{
    (void)arg;
    while(1){
        printf("ljn最帅！\r\n");
        osDelay(100);
        unsigned int signal_press=GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_GPIO_DIR_IN);
        printf("signal_press=%d\r\n",signal_press);
    }

    return NULL;
}
static void Child_lock_ex1(void)
{
    osThreadAttr_t attr;
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_IO_FUNC_GPIO_5_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_5,WIFI_IOT_IO_PULL_UP);
    
    
    
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