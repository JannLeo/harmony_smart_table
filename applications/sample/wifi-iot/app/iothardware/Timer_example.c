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
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

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
